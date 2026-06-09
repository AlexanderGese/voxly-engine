#include "jobsys_parallel.h"

#include <stdlib.h>
#include "../assert.h"

// one of these per submitted chunk. carries the sub-range and the user's body.
// it's malloc'd by the dispatcher and freed by the trampoline once the body
// returns -- jobs are plain fn+arg so this is how we smuggle the extra fields.
// yes that's a malloc per chunk; the chunk count is small (a few per worker) and
// the body does real work, so it disappears in the noise. a pool would shave it
// but i measured and it wasnt worth the lifetime headache.
typedef struct {
    jobsys_range_fn body;
    void           *arg;
    int             begin;
    int             end;
} range_desc;

// the actual jobsys_fn. unpacks the descriptor, runs the body, frees it.
static void range_trampoline(void *p, int worker) {
    range_desc *d = (range_desc *)p;
    d->body(d->begin, d->end, d->arg, worker);
    free(d);
}

int jobsys_parallel_grain(const jobsys_pool *p, int count, int per_worker) {
    if (count <= 0) return 1;
    if (per_worker < 1) per_worker = 1;
    // aim for nworkers*per_worker chunks so the deques have something to steal.
    int want_chunks = p->nworkers * per_worker;
    if (want_chunks < 1) want_chunks = 1;
    int grain = (count + want_chunks - 1) / want_chunks;   // ceil
    return grain < 1 ? 1 : grain;
}

jobsys_handle jobsys_parallel_for(jobsys_pool *p, int count, int grain,
                                  jobsys_range_fn body, void *arg,
                                  jobsys_prio prio) {
    VX_ASSERT(body != NULL);

    if (count <= 0) {
        // nothing to do -- hand back a fence that's already signalled so the
        // caller's wait() returns instantly without special-casing.
        return jobsys_fence_alloc(&p->fences, 0);
    }

    if (grain <= 0) grain = jobsys_parallel_grain(p, count, 4);

    // how many chunks will we actually emit? ceil(count/grain).
    int nchunks = (count + grain - 1) / grain;

    // mint the fence preloaded with the chunk count, then fan out. allocating it
    // up front (vs add-as-we-go) avoids a race where an early chunk finishes and
    // drives the count to zero before we've queued the rest.
    jobsys_handle h = jobsys_fence_alloc(&p->fences, nchunks);
    if (!jobsys_handle_valid(h)) {
        // fence table exhausted. run it inline rather than fail the caller --
        // ugly, but a stall beats a dropped batch, and this basically never hits.
        for (int b = 0; b < count; b += grain) {
            int e = b + grain; if (e > count) e = count;
            body(b, e, arg, -1);
        }
        return jobsys_handle_null();
    }

    for (int b = 0; b < count; b += grain) {
        int e = b + grain;
        if (e > count) e = count;
        range_desc *d = malloc(sizeof(*d));
        d->body  = body;
        d->arg   = arg;
        d->begin = b;
        d->end   = e;
        jobsys_submit(p, range_trampoline, d, h, prio, 0);
    }

    return h;
}

void jobsys_parallel_for_sync(jobsys_pool *p, int count, int grain,
                              jobsys_range_fn body, void *arg, jobsys_prio prio) {
    jobsys_handle h = jobsys_parallel_for(p, count, grain, body, arg, prio);
    if (jobsys_handle_valid(h)) {
        jobsys_wait(p, h);   // help-drains while waiting, recycles the fence
    }
    // invalid handle path means it already ran inline (fence exhaustion), nothing
    // to wait on.
}
