#ifndef UTIL_JOBSYS_H
#define UTIL_JOBSYS_H
// the job system. a fixed pool of worker threads, each with its own work-
// stealing deque (jobsys_deque), a shared overflow queue for spillover and
// cross-thread submits (jobsys_overflow), a fence table for "wait until done"
// (jobsys_fence), and a continuation pool for chains (jobsys_chain).
//
// the whole point is to get chunk gen/light/mesh off the main thread without the
// cooperative time-slicing the loader/ does today. submit work, get a handle,
// keep rendering, and pick the results up when the fence signals. the main
// thread stays the only one touching gl -- those jobs ride the main-only lane.
//
// jobsys_pool pool;
// jobsys_init(&pool, 0);                 // 0 = autodetect cores
// jobsys_handle h = jobsys_fence_alloc(&pool.fences, 0);
// jobsys_submit(&pool, mesh_chunk, c, h, JOBSYS_PRIO_NORMAL, 0);
// ... render the frame ...
// if (jobsys_fence_done(&pool.fences, h)) collect();   // or wait()
// jobsys_shutdown(&pool);
#include <pthread.h>
#include "jobsys_types.h"
#include "jobsys_deque.h"
#include "jobsys_overflow.h"
#include "jobsys_fence.h"
#include "jobsys_chain.h"
#include "jobsys_trace.h"
// live counters for the debug overlay. all relaxed atomics, read racily.
typedef struct {
    jat_u64 submitted;     // jobs handed to submit()
    jat_u64 executed;      // jobs actually run to completion
    jat_u64 stolen;        // jobs a worker took from someone else's deque
    jat_u64 overflowed;    // jobs that spilled to the shared queue
    jat_u64 main_ran;      // main-only jobs drained on the main thread
} jobsys_stats;
typedef struct jobsys_pool jobsys_pool;
// per-worker state. the pool owns an array of these. kept here (not hidden in
// the .c) so the worker loop and the debug overlay can both see it.
typedef struct {
    jobsys_pool   *pool;
    jobsys_deque   deque;
    pthread_t      thread;
    int            id;          // 0..nworkers-1
    uint64_t       rng;         // per-worker prng for randomised victim picking
#ifdef JOBSYS_TRACE
    jobsys_trace   trace;       // recent job timings, debug builds only
#endif
} jobsys_worker;
struct jobsys_pool {
    jobsys_worker     workers[JOBSYS_MAX_WORKERS];
    int               nworkers;

    jobsys_overflow   overflow;
    jobsys_fence_pool fences;
    jobsys_chain_pool chains;
    jobsys_stats      stats;

    // sleeping workers park on this when theres no work, instead of spinning a
    // core flat. a submit broadcasts to wake them. classic condvar handshake.
    pthread_mutex_t   sleep_mtx;
    pthread_cond_t    sleep_cv;
    jat_i32           waiters;     // how many workers are parked right now

    jat_i32           running;     // 1 while workers should keep looping
    jat_u32           rr;          // round-robin cursor for external submits
}
;
// spin up the pool. nworkers<=0 means autodetect (cores-1, clamped to
// [1, JOBSYS_MAX_WORKERS]); we leave a core for the main thread. returns 0 ok.
int  jobsys_init(jobsys_pool *p, int nworkers);
// signal every worker to drain and exit, join them, free everything. safe to
// call once. any unfinished jobs are run to completion first so fences settle.
void jobsys_shutdown(jobsys_pool *p);
// submit a job. if called from a worker it goes on that worker's local deque
// (hot path, lifo); from any other thread it goes to the overflow queue. `fence`
// is a handle to signal on completion or a null handle for fire-and-forget.
// returns 0 on success. wakes a sleeping worker if any.
int  jobsys_submit(jobsys_pool *p, jobsys_fn fn, void *arg,
                   jobsys_handle fence, jobsys_prio prio, uint16_t flags);
// submit a job AND a continuation that fires when it finishes. the continuation
// inherits the same fence (so the fence counts the whole chain -- bump it by the
// chain length first, or pass a null fence and let each link self-signal).
// returns 0 ok, -1 if the chain pool is full (then `first` runs alone).
int  jobsys_submit_chain(jobsys_pool *p, jobsys_fn first, void *first_arg,
                         jobsys_fn then, void *then_arg,
                         jobsys_handle fence, jobsys_prio prio);
// drain the main-only lane. call once per frame from the main (gl) thread. runs
// at most `budget` jobs so a flood of uploads cant blow the frame time; pass 0
// for "everything pending". returns how many ran.
int  jobsys_run_main(jobsys_pool *p, int budget);
// block the CALLING thread until the fence signals, helping drain work while it
// waits so we dont leave cores idle (and dont deadlock if the caller is the only
// thing that can produce the work the fence depends on). main-thread friendly.
void jobsys_wait(jobsys_pool *p, jobsys_handle h);
// snapshot of the counters. cheap, racy, debug only.
jobsys_stats jobsys_get_stats(const jobsys_pool *p);
#endif
