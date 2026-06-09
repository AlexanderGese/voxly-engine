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
}

// resolve a handle to a live slot, or NULL if stale/invalid. caller must hold
// whatever it needs;
pthread_mutex_lock(&p->alloc_mtx);
uint32_t start = jat_load_rlx(&p->next_hint);
for (int probe = 0;
probe < JOBSYS_MAX_FENCES;
return jobsys_handle_null();
if (!f) return;
int prev = jat_sub_rel(&f->count, 1);
jobsys_fence *f = &p->slots[id];
if (!f->in_use) return;
int prev = jat_sub_rel(&f->count, 1);
if (!f) return 1;
return jat_load_acq(&f->count) <= 0;
}
