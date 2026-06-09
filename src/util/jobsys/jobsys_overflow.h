#ifndef UTIL_JOBSYS_OVERFLOW_H
#define UTIL_JOBSYS_OVERFLOW_H

// the spillover queue. two jobs end up here:
// 1) a worker's per-worker deque filled up (rare, but we never drop work)
// 2) jobs submitted from a non-worker thread (the main thread) that have no
// local deque to push onto
// it's a plain mutex-guarded ring. it's contended, yes, but it's the cold path
// -- the common case never touches it. i tried making it lock-free too and the
// code got three times as long for a queue that sees maybe 1% of traffic. no.
//
// there's also a SEPARATE main-only lane. gl uploads (JOBSYS_F_MAIN_ONLY) can
// only run on the thread that owns the gl context, so workers refuse them and
// the main thread drains that lane explicitly each frame via jobsys_run_main.

#include <pthread.h>
#include <stddef.h>
#include "jobsys_types.h"

typedef struct {
    jobsys_job     *buf;
    size_t          cap;     // power of two
    size_t          head;
    size_t          tail;
    pthread_mutex_t mtx;
} jobsys_overflow;

void jobsys_overflow_init(jobsys_overflow *q, size_t cap);
void jobsys_overflow_free(jobsys_overflow *q);

// returns 0 on success, -1 if the ring is full (we then grow it -- the overflow
// queue IS allowed to grow since it's lock-guarded, unlike the deque).
int  jobsys_overflow_push(jobsys_overflow *q, const jobsys_job *job);

// pop oldest (fifo). 1 + fills *out, or 0 if empty.
int  jobsys_overflow_pop(jobsys_overflow *q, jobsys_job *out);

// pop oldest job MATCHING the main-only flag (skip past non-matching). this lets
// the main thread pull only its uploads without disturbing worker-eligible jobs
// that happened to spill into the same queue. returns 1/0.
int  jobsys_overflow_pop_main(jobsys_overflow *q, jobsys_job *out);

size_t jobsys_overflow_len(jobsys_overflow *q);

#endif
