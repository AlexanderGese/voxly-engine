#ifndef UTIL_JOBSYS_TRACE_H
#define UTIL_JOBSYS_TRACE_H

// lightweight per-worker job tracing. each worker keeps a small ring of its most
// recent job timings (start, duration, whether it was stolen). the overlay reads
// these to flag long jobs -- a single 8ms mesh job stalling a worker is the kind
// of thing that hides in an averaged counter but jumps out of a trace.
//
// this is opt-in and cheap: a timer_now_us() before and after each job, written
// into a thread-local ring with no locks (only the owning worker writes, the
// overlay reads racily and accepts the occasional torn sample -- it's a debug
// view, not telemetry we bet money on). disabled entirely if JOBSYS_TRACE is
// not defined, so release builds pay nothing.

#include <stdint.h>
#include "jobsys_types.h"

// samples kept per worker. power of two so the write index masks cheaply. 256
// at ~50us/job is ~12ms of history, enough to catch a hitch in the act.
#define JOBSYS_TRACE_RING  256

typedef struct {
    uint64_t start_us;     // when the job began
    uint32_t dur_us;       // how long it ran (capped at ~4s, plenty)
    uint16_t prio;         // jobsys_prio it carried
    uint16_t stolen;       // 1 if this worker stole the job, 0 if local
} jobsys_trace_sample;

typedef struct {
    jobsys_trace_sample ring[JOBSYS_TRACE_RING];
    uint32_t            head;     // next write slot, owner-only
    uint64_t            longest_us;   // worst job seen, sticky high-water mark
    uint64_t            total_us;     // sum of durations, for utilisation calc
    uint64_t            njobs;        // count, pairs with total_us
} jobsys_trace;

void jobsys_trace_init(jobsys_trace *t);

// record one completed job. called by the worker right after the body returns.
// start_us is from before the call, end_us from after. owner thread only.
void jobsys_trace_record(jobsys_trace *t, uint64_t start_us, uint64_t end_us,
                         jobsys_prio prio, int stolen);

// fraction of wall time this worker spent actually running jobs over the span
// the ring covers, 0..1. <1 means it was idle/stealing/sleeping the rest. racy.
double jobsys_trace_utilisation(const jobsys_trace *t);

// the longest single job duration on record, in microseconds. the overlay turns
// this red past a threshold. resettable so you can watch a specific window.
uint64_t jobsys_trace_longest(const jobsys_trace *t);
void     jobsys_trace_reset_longest(jobsys_trace *t);

#endif
