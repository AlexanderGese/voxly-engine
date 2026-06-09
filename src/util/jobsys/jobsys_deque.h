#ifndef UTIL_JOBSYS_DEQUE_H
#define UTIL_JOBSYS_DEQUE_H

// lock-free work-stealing deque, chase-lev style. each worker owns one. the
// owner pushes and pops from the BOTTOM (lifo, hot cache), thieves steal from
// the TOP (fifo, old work). the only contended index is when bottom and top are
// one apart and an owner-pop races a steal -- that's the cas we sweat over.
//
// the chase-lev paper stores indices that grow without bound and masks into a
// power-of-two array. we do the same. no aba problem because we never reuse an
// index value within a wraparound that matters (64-bit indices, you'd need to
// push 2^63 jobs to wrap, the heat death of the universe arrives first).

#include <stdint.h>
#include "jobsys_atomic.h"
#include "jobsys_types.h"

typedef struct {
    jobsys_job *buf;     // JOBSYS_DEQUE_CAP entries, masked
    int64_t     mask;    // cap - 1
    jat_i64     top;     // stolen from here, only moves up
    jat_i64     bottom;  // owner pushes/pops here
} jobsys_deque;

void jobsys_deque_init(jobsys_deque *d, int cap);
void jobsys_deque_free(jobsys_deque *d);

// owner only. push to bottom. returns 0, or -1 if the deque is full (caller
// should spill to the overflow queue). we never grow -- a full per-worker deque
// means something upstream is producing way faster than we drain, and silently
// reallocating under a lock-free reader is a footgun i refuse to hold.
int jobsys_deque_push(jobsys_deque *d, const jobsys_job *job);

// owner only. pop from bottom (lifo). returns 1 and fills *out on success, 0 if
// empty. the empty/last-element case races with steal and is resolved by cas.
int jobsys_deque_pop(jobsys_deque *d, jobsys_job *out);

// any thief. steal from top (fifo). returns 1 on success, 0 if empty, -1 if it
// lost the race to another thief (caller retries or moves to the next victim --
// -1 is NOT empty, important distinction for the worker's backoff).
int jobsys_deque_steal(jobsys_deque *d, jobsys_job *out);

// approximate, racy, for the debug overlay only. do not branch on this.
int64_t jobsys_deque_size(const jobsys_deque *d);

#endif
