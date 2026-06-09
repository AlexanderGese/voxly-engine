#include "jobsys.h"
#include "jobsys_worker.h"

#include <unistd.h>
#include <string.h>
#include "../log.h"
#include "../assert.h"

// how many cpus do we have. cores-1 by default so the main/render thread keeps a
// core to itself. if sysconf is unhelpful we assume a modest 4.
static int detect_workers(void) {
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    if (n < 1) n = 4;
    n -= 1;                       // leave one for the main thread
    if (n < 1) n = 1;
    if (n > JOBSYS_MAX_WORKERS) n = JOBSYS_MAX_WORKERS;
    return (int)n;
}

int jobsys_init(jobsys_pool *p, int nworkers) {
    memset(&p->stats, 0, sizeof(p->stats));

    p->nworkers = nworkers > 0 ? nworkers : detect_workers();
    if (p->nworkers > JOBSYS_MAX_WORKERS) p->nworkers = JOBSYS_MAX_WORKERS;

    jobsys_overflow_init(&p->overflow, 256);
    jobsys_fence_pool_init(&p->fences);
    jobsys_chain_pool_init(&p->chains);

    pthread_mutex_init(&p->sleep_mtx, NULL);
    pthread_cond_init(&p->sleep_cv, NULL);
    atomic_store_explicit(&p->waiters, 0, memory_order_relaxed);
    atomic_store_explicit(&p->rr, 0, memory_order_relaxed);
    atomic_store_explicit(&p->running, 1, memory_order_relaxed);

    for (int i = 0; i < p->nworkers; i++) {
        jobsys_worker *w = &p->workers[i];
        w->pool = p;
        w->id   = i;
        // seed each worker's prng distinctly so victim choices arent correlated.
        w->rng  = 0x9e3779b97f4a7c15ull ^ ((uint64_t)(i + 1) * 0xff51afd7ed558ccdull);
        jobsys_deque_init(&w->deque, JOBSYS_DEQUE_CAP);
#ifdef JOBSYS_TRACE
        jobsys_trace_init(&w->trace);
#endif
    }

    // start threads last, once every deque exists -- a worker can try to steal
    // from any sibling the instant it wakes, so they must all be live first.
    for (int i = 0; i < p->nworkers; i++) {
        int rc = pthread_create(&p->workers[i].thread, NULL,
                                jobsys_worker_main, &p->workers[i]);
        if (rc != 0) {
            LOGE("jobsys: failed to spawn worker %d (rc=%d)", i, rc);
            // unwind: stop whatever we started, then bail.
            atomic_store_explicit(&p->running, 0, memory_order_relaxed);
            pthread_cond_broadcast(&p->sleep_cv);
            for (int j = 0; j < i; j++) pthread_join(p->workers[j].thread, NULL);
            return -1;
        }
    }

    LOGI("jobsys: %d workers up", p->nworkers);
    return 0;
}

void jobsys_shutdown(jobsys_pool *p) {
    // tell workers to stop, then wake any that are parked so they see it.
    atomic_store_explicit(&p->running, 0, memory_order_release);
    pthread_mutex_lock(&p->sleep_mtx);
    pthread_cond_broadcast(&p->sleep_cv);
    pthread_mutex_unlock(&p->sleep_mtx);

    for (int i = 0; i < p->nworkers; i++) {
        pthread_join(p->workers[i].thread, NULL);
        jobsys_deque_free(&p->workers[i].deque);
    }

    jobsys_overflow_free(&p->overflow);
    jobsys_fence_pool_free(&p->fences);
    jobsys_chain_pool_free(&p->chains);
    pthread_mutex_destroy(&p->sleep_mtx);
    pthread_cond_destroy(&p->sleep_cv);

    LOGI("jobsys: down. ran %llu jobs (%llu stolen)",
         (unsigned long long)jat_load_rlx(&p->stats.executed),
         (unsigned long long)jat_load_rlx(&p->stats.stolen));
}

// is the calling thread one of our workers? if so return it, else NULL. cheap
// linear scan over a tiny array; we dont bother with pthread tls for this.
static jobsys_worker *current_worker(jobsys_pool *p) {
    pthread_t self = pthread_self();
    for (int i = 0; i < p->nworkers; i++) {
        if (pthread_equal(p->workers[i].thread, self)) return &p->workers[i];
    }
    return NULL;
}

// nudge one parked worker awake. we signal (not broadcast) for a single submit
// so we dont thundering-herd every worker for one job.
static void wake_one(jobsys_pool *p) {
    if (jat_load_rlx(&p->waiters) > 0) {
        pthread_mutex_lock(&p->sleep_mtx);
        pthread_cond_signal(&p->sleep_cv);
        pthread_mutex_unlock(&p->sleep_mtx);
    }
}

int jobsys_submit(jobsys_pool *p, jobsys_fn fn, void *arg,
                  jobsys_handle fence, jobsys_prio prio, uint16_t flags) {
    VX_ASSERT(fn != NULL);

    jobsys_job job;
    job.fn    = fn;
    job.arg   = arg;
    job.fence = jobsys_handle_valid(fence) ? fence.id : -1;
    job.cont  = -1;
    job.prio  = (uint16_t)prio;
    job.flags = flags;

    jat_add_rlx(&p->stats.submitted, 1);

    // main-only jobs always go to the shared queue; only the main thread will
    // pull them (jobsys_run_main). they must never sit on a worker deque.
    if (flags & JOBSYS_F_MAIN_ONLY) {
        jobsys_overflow_push(&p->overflow, &job);
        wake_one(p);
        return 0;
    }

    jobsys_worker *self = current_worker(p);
    if (self) {
        // local push, hot path. on overflow spill to the shared queue.
        if (jobsys_deque_push(&self->deque, &job) != 0) {
            jobsys_overflow_push(&p->overflow, &job);
            jat_add_rlx(&p->stats.overflowed, 1);
        }
    } else {
        jobsys_overflow_push(&p->overflow, &job);
    }
    wake_one(p);
    return 0;
}

int jobsys_submit_chain(jobsys_pool *p, jobsys_fn first, void *first_arg,
                        jobsys_fn then, void *then_arg,
                        jobsys_handle fence, jobsys_prio prio) {
    VX_ASSERT(first && then);

    // build the continuation first and stash it; the predecessor will carry its
    // index. the continuation inherits the fence so the whole chain counts.
    jobsys_job cont;
    cont.fn    = then;
    cont.arg   = then_arg;
    cont.fence = jobsys_handle_valid(fence) ? fence.id : -1;
    cont.cont  = -1;
    cont.prio  = (uint16_t)prio;
    cont.flags = JOBSYS_F_NONE;

    int cidx = jobsys_chain_store(&p->chains, &cont);
    if (cidx < 0) {
        // chain pool full: degrade gracefully -- run them as two independent
        // submits. ordering isnt guaranteed then, but the caller's fence (which
        // should have been bumped by 2) still settles correctly.
        jobsys_submit(p, first, first_arg, fence, prio, 0);
        jobsys_submit(p, then,  then_arg,  fence, prio, 0);
        return -1;
    }

    jobsys_job head;
    head.fn    = first;
    head.arg   = first_arg;
    head.fence = jobsys_handle_valid(fence) ? fence.id : -1;
    head.cont  = cidx;
    head.prio  = (uint16_t)prio;
    head.flags = JOBSYS_F_NONE;

    jat_add_rlx(&p->stats.submitted, 1);

    jobsys_worker *self = current_worker(p);
    if (self) {
        if (jobsys_deque_push(&self->deque, &head) != 0) {
            jobsys_overflow_push(&p->overflow, &head);
            jat_add_rlx(&p->stats.overflowed, 1);
        }
    } else {
        jobsys_overflow_push(&p->overflow, &head);
    }
    wake_one(p);
    return 0;
}

int jobsys_run_main(jobsys_pool *p, int budget) {
    int ran = 0;
    jobsys_job job;
    while (budget <= 0 || ran < budget) {
        if (!jobsys_overflow_pop_main(&p->overflow, &job)) break;
        // worker id -1 marks "ran on the main thread" -- jobs that index per-
        // worker scratch must check for this. uploads dont, they only touch gl.
        job.fn(job.arg, -1);
        if (job.fence >= 0) jobsys_fence_signal_by_id(&p->fences, job.fence);
        jat_add_rlx(&p->stats.main_ran, 1);
        ran++;
    }
    return ran;
}

void jobsys_wait(jobsys_pool *p, jobsys_handle h) {
    if (!jobsys_handle_valid(h)) return;

    // help-while-waiting. if the caller is the main thread (or any thread) we
    // dont just park -- we pull jobs and run them so the fence's own work makes
    // progress. crucial: if the main thread blocks naively on a fence whose jobs
    // are still queued and all workers are asleep, nobody wakes them and we hang.
    jobsys_worker *self = current_worker(p);
    while (!jobsys_fence_done(&p->fences, h)) {
        jobsys_job job;
        int got = 0;

        if (self) {
            got = jobsys_worker_acquire(self, &job);
        } else {
            // non-worker (main thread): drain main-only first, then general work.
            if (jobsys_overflow_pop_main(&p->overflow, &job)) {
                job.fn(job.arg, -1);
                if (job.fence >= 0) jobsys_fence_signal_by_id(&p->fences, job.fence);
                jat_add_rlx(&p->stats.main_ran, 1);
                continue;
            }
            got = jobsys_overflow_pop(&p->overflow, &job);
            if (got && (job.flags & JOBSYS_F_MAIN_ONLY)) {
                // not ours, shouldnt happen after the pop_main above, but be safe
                jobsys_overflow_push(&p->overflow, &job);
                got = 0;
            }
        }

        if (got) {
            if (self) {
                jobsys_worker_execute(p, self, &job);
            } else {
                job.fn(job.arg, -1);
                if (job.fence >= 0) jobsys_fence_signal_by_id(&p->fences, job.fence);
                jat_add_rlx(&p->stats.executed, 1);
            }
        } else {
            // nothing to help with -- the remaining work is in flight on workers.
            // fall back to a real condvar wait so we dont spin a core. this also
            // recycles the fence slot on the way out.
            jobsys_fence_wait(&p->fences, h);
            return;
        }
    }

    // fence already done by the time we got here; release the slot ourselves so
    // it doesnt leak (jobsys_fence_wait would have, but we short-circuited it).
    jobsys_fence_release(&p->fences, h);
}

jobsys_stats jobsys_get_stats(const jobsys_pool *p) {
    // copy out the atomics into a plain snapshot. racy by nature, debug only.
    jobsys_stats s;
    s.submitted  = jat_load_rlx(&p->stats.submitted);
    s.executed   = jat_load_rlx(&p->stats.executed);
    s.stolen     = jat_load_rlx(&p->stats.stolen);
    s.overflowed = jat_load_rlx(&p->stats.overflowed);
    s.main_ran   = jat_load_rlx(&p->stats.main_ran);
    return s;
}
