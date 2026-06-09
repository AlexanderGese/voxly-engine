#ifndef UTIL_JOBSYS_TYPES_H
#define UTIL_JOBSYS_TYPES_H

// shared plain-data for the job system. this is the vocabulary every other
// jobsys file speaks. the loader/ pipeline used to slice its work cooperatively
// on the main thread (see loader_types.h, "the async is cooperative"); this is
// the real thread pool i promised to bolt on later. chunk gen/light/mesh become
// jobs, the worker threads chew through them, and a fence tells the main thread
// when a batch is done so it can do the gl upload.

#include <stdint.h>
#include <stddef.h>

// hard cap on workers. we never spin more than this regardless of core count --
// a voxel mesher is memory bound and past ~12 threads we just thrash cache. the
// pool clamps to min(detected, this).
#define JOBSYS_MAX_WORKERS   16

// per-worker deque capacity. power of two, masked. 4096 pending jobs per thread
// is absurdly generous; if we ever overflow it the submit falls back to the
// shared overflow queue instead of dropping work.
#define JOBSYS_DEQUE_CAP     4096

// fences live in a fixed table so a handle is just an index+generation. 256 is
// plenty -- the loader uses maybe a dozen in flight (one per stage per frame).
#define JOBSYS_MAX_FENCES    256

// the function a job runs. `arg` is whatever you stuffed in the job, `worker` is
// the id of the thread executing it (0..nworkers-1) so jobs can index per-thread
// scratch without a tls lookup. keep these SHORT -- a job that blocks parks a
// whole worker. if you need to wait, split it and chain a fence.
typedef void (*jobsys_fn)(void *arg, int worker);

// priority. the deque is lifo for cache locality but when a worker has nothing
// local and steals, it prefers high-prio victims. mostly the loader tags upload
// prep HIGH and speculative far-chunk gen LOW.
typedef enum {
    JOBSYS_PRIO_LOW    = 0,
    JOBSYS_PRIO_NORMAL = 1,
    JOBSYS_PRIO_HIGH   = 2,
    JOBSYS_PRIO_COUNT
} jobsys_prio;

// a single unit of work. deliberately fits in a cache line-ish footprint so the
// deque stays dense. `fence` is the counter to decrement when this job finishes
// (>=0), or -1 for fire-and-forget. `cont` is an optional continuation job
// submitted only after this one completes -- that's how we build chains without
// a job blocking on another.
typedef struct {
    jobsys_fn   fn;
    void       *arg;
    int32_t     fence;     // fence id to signal on completion, -1 = none
    int32_t     cont;      // continuation job index in the chain pool, -1 = none
    uint16_t    prio;      // jobsys_prio, kept narrow
    uint16_t    flags;
} jobsys_job;

// job flags. mostly diagnostics + the one real behavioural bit (MAIN_ONLY).
enum {
    JOBSYS_F_NONE      = 0,
    JOBSYS_F_MAIN_ONLY = 1 << 0,  // must run on the main thread (gl upload etc)
    JOBSYS_F_LONG      = 1 << 1,  // hint: heavy job, dont batch-steal a pile of these
};

// a handle the caller holds onto. it's an index into the fence table plus a
// generation so a stale handle from a recycled slot doesnt accidentally match a
// live fence. compare with jobsys_handle_eq, dont memcmp (padding).
typedef struct {
    int32_t  id;    // fence slot, -1 = invalid/null handle
    uint32_t gen;   // generation of that slot when the handle was minted
} jobsys_handle;

static inline jobsys_handle jobsys_handle_null(void) {
    jobsys_handle h = { -1, 0 };
    return h;
}

static inline int jobsys_handle_valid(jobsys_handle h) {
    return h.id >= 0;
}

static inline int jobsys_handle_eq(jobsys_handle a, jobsys_handle b) {
    return a.id == b.id && a.gen == b.gen;
}

#endif
