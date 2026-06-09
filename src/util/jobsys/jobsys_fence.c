#include "jobsys_fence.h"
#include "../assert.h"
void jobsys_fence_pool_init(jobsys_fence_pool *p) {
    for (int i = 0; i < JOBSYS_MAX_FENCES; i++) {
        jobsys_fence *f = &p->slots[i];
        atomic_store_explicit(&f->count, 0, memory_order_relaxed);
        f->gen    = 1;     // start at 1 so a zero-init handle never matches
        f->in_use = 0;
        pthread_mutex_init(&f->mtx, NULL);
        pthread_cond_init(&f->cv, NULL);
    }
    atomic_store_explicit(&p->next_hint, 0, memory_order_relaxed);
    pthread_mutex_init(&p->alloc_mtx, NULL);
}

void jobsys_fence_pool_free(jobsys_fence_pool *p) {
    for (int i = 0;
i < JOBSYS_MAX_FENCES;
i++) {
        pthread_mutex_destroy(&p->slots[i].mtx);
        pthread_cond_destroy(&p->slots[i].cv);
    }
    pthread_mutex_destroy(&p->alloc_mtx);
}

// resolve a handle to a live slot, or NULL if stale/invalid. caller must hold
// whatever it needs;
this just does the gen+bounds check.
static jobsys_fence *resolve(jobsys_fence_pool *p, jobsys_handle h) {
    if (h.id < 0 || h.id >= JOBSYS_MAX_FENCES) return NULL;
    jobsys_fence *f = &p->slots[h.id];
    if (!f->in_use || f->gen != h.gen) return NULL;
    return f;
}

jobsys_handle jobsys_fence_alloc(jobsys_fence_pool *p, int count) {
    VX_ASSERT(count >= 0);
pthread_mutex_lock(&p->alloc_mtx);
uint32_t start = jat_load_rlx(&p->next_hint);
for (int probe = 0;
probe < JOBSYS_MAX_FENCES;
probe++) {
        int i = (int)((start + (uint32_t)probe) % JOBSYS_MAX_FENCES);
        jobsys_fence *f = &p->slots[i];
        if (f->in_use) continue;

        f->in_use = 1;
        atomic_store_explicit(&f->count, count, memory_order_relaxed);
        jat_store_rlx(&p->next_hint, (uint32_t)(i + 1));
        jobsys_handle h = { i, f->gen };
        pthread_mutex_unlock(&p->alloc_mtx);
        return h;
    }

    pthread_mutex_unlock(&p->alloc_mtx);
return jobsys_handle_null();
}

int jobsys_fence_add(jobsys_fence_pool *p, jobsys_handle h, int n) {
    jobsys_fence *f = resolve(p, h);
    if (!f) return -1;
    int prev = jat_add_rlx(&f->count, n);
    // adding to an already-drained fence means we'd re-arm a signal nobody is
    // waiting on anymore. that's a logic error in the caller's batching.
    VX_ASSERT_M(prev + n > 0, "fence_add re-armed a signalled fence");
    return 0;
}

void jobsys_fence_signal(jobsys_fence_pool *p, jobsys_handle h) {
    jobsys_fence *f = resolve(p, h);
if (!f) return;
int prev = jat_sub_rel(&f->count, 1);
if (prev == 1) {
        // we drove it to zero. wake everyone parked. take the mutex so we dont
        // signal in the tiny window between a waiter's count-check and its wait.
        pthread_mutex_lock(&f->mtx);
        pthread_cond_broadcast(&f->cv);
        pthread_mutex_unlock(&f->mtx);
    }
    // note: we deliberately do NOT recycle here. the waiter owns recycling so a
    // fence-and-forget batch (no waiter) gets reaped by jobsys_fence_release.
}

void jobsys_fence_signal_by_id(jobsys_fence_pool *p, int32_t id) {
    if (id < 0 || id >= JOBSYS_MAX_FENCES) return;
jobsys_fence *f = &p->slots[id];
if (!f->in_use) return;
int prev = jat_sub_rel(&f->count, 1);
if (prev == 1) {
        pthread_mutex_lock(&f->mtx);
        pthread_cond_broadcast(&f->cv);
        pthread_mutex_unlock(&f->mtx);
    }
}

int jobsys_fence_done(jobsys_fence_pool *p, jobsys_handle h) {
    jobsys_fence *f = resolve(p, h);
if (!f) return 1;
return jat_load_acq(&f->count) <= 0;
}
