#ifndef UTIL_JOBSYS_FENCE_H
#define UTIL_JOBSYS_FENCE_H

// fences are just counters with a waiter. you mint one with a count == number of
// jobs in the batch, every job decrements it on completion, and when it hits
// zero anyone parked on it wakes up. this is how the main thread waits for "all
// of this frame's mesh jobs" without polling or busy-spinning a core.
//
// the slots live in a fixed table so a jobsys_handle is index + generation. when
// a fence is recycled the generation bumps, which invalidates any stale handle
// still floating around (you'd be amazed how often a frame holds a handle one
// tick too long). all the public ops validate the generation first.

#include <pthread.h>
#include <stdint.h>
#include "jobsys_atomic.h"
#include "jobsys_types.h"

typedef struct {
    jat_i32         count;      // outstanding jobs. zero == signalled.
    uint32_t        gen;        // bumped on recycle, matched against handle.gen
    int             in_use;     // slot allocated (gen-matched handles only)
    pthread_mutex_t mtx;        // guards the condvar handshake, not the count
    pthread_cond_t  cv;         // waiters park here until count hits zero
} jobsys_fence;

typedef struct {
    jobsys_fence    slots[JOBSYS_MAX_FENCES];
    jat_u32         next_hint;  // round-robin search start, reduces scan length
    pthread_mutex_t alloc_mtx;  // serialises slot allocation only
} jobsys_fence_pool;

void jobsys_fence_pool_init(jobsys_fence_pool *p);
void jobsys_fence_pool_free(jobsys_fence_pool *p);

// grab a fence preloaded with `count`. returns a null handle if the table is
// exhausted (caller should drain something and retry, this is rare). count may
// be 0 -- a pre-signalled fence, occasionally handy as a chain anchor.
jobsys_handle jobsys_fence_alloc(jobsys_fence_pool *p, int count);

// add to the count of a live fence. used when you append jobs to a batch after
// minting its fence. returns -1 if the handle is stale. bumping a fence that has
// already hit zero is a bug (it was someone's signal to recycle) -- asserts.
int jobsys_fence_add(jobsys_fence_pool *p, jobsys_handle h, int n);

// decrement by one. when it reaches zero we broadcast to waiters. the worker
// calls this as each job retires. stale handle -> no-op (the fence outlived its
// batch and got recycled, the job is just late, nobody's listening).
void jobsys_fence_signal(jobsys_fence_pool *p, jobsys_handle h);

// decrement by slot id only, ignoring generation. the worker uses this because a
// job stores just the fence id (see jobsys_types.h) to stay narrow. behaves like
// jobsys_fence_signal otherwise -- broadcasts on the zero transition. id out of
// range or a slot that isnt in_use is a no-op.
void jobsys_fence_signal_by_id(jobsys_fence_pool *p, int32_t id);

// non-blocking check. 1 if signalled (count==0) or the handle is already stale,
// 0 if still outstanding.
int jobsys_fence_done(jobsys_fence_pool *p, jobsys_handle h);

// block until the fence is signalled. returns immediately if already done or
// stale. NOTE: this parks the calling thread -- never call it FROM a worker on a
// fence that worker is supposed to help drain, you'll deadlock the pool. the
// pool's own wait_help() path is the one that drains while waiting.
void jobsys_fence_wait(jobsys_fence_pool *p, jobsys_handle h);

// release the slot back to the table and bump its generation. you only call this
// if you minted a fence and then decided NOT to wait on it; the normal path
// recycles automatically on the wait. double-free is caught by the gen check.
void jobsys_fence_release(jobsys_fence_pool *p, jobsys_handle h);

#endif
