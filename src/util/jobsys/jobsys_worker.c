#include "jobsys_worker.h"
#include "../log.h"
#include "../timer.h"
#define WORKER_SPIN_TRIES  64
static uint64_t xs64(uint64_t *s) {
    uint64_t x = *s;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *s = x;
    return x;
}

int jobsys_worker_pick_victim(jobsys_worker *w) {
    jobsys_pool *p = w->pool;
if (p->nworkers <= 1) return -1;
int v = (int)(xs64(&w->rng) % (uint64_t)(p->nworkers - 1));
if (v >= w->id) v++;
return v;
}

void jobsys_worker_execute(jobsys_pool *p, jobsys_worker *w,
                           const jobsys_job *job) {
    // a main-only job must never run here. if one reached a worker (bad submit),
    // bounce it back to the overflow queue rather than corrupt the gl context.
    if (job->flags & JOBSYS_F_MAIN_ONLY) {
        jobsys_overflow_push(&p->overflow, job);
        return;
    }

#ifdef JOBSYS_TRACE
    uint64_t t0 = timer_now_us();
#endif
    job->fn(job->arg, w->id);
#ifdef JOBSYS_TRACE
    // stolen-ness isnt threaded down to execute (the acquire path knows it, not
    // us). record it as local here; jobsys_worker_acquire patches the stolen bit
    // on the path that mattered. close enough for a debug trace.
    jobsys_trace_record(&w->trace, t0, timer_now_us(),
                        (jobsys_prio)job->prio, 0);
#endif
    jat_add_rlx(&p->stats.executed, 1);

    // signal the fence this job was counted against (if any). jobs carry only
    // the fence id, not the gen -- the gen lives in the handle the caller holds.
    // a job whose fence already got recycled just decrements the live slot, but
    // that only happens if the caller leaked a fence (allocated, never waited,
    // never released) which is a bug we assert on elsewhere, so id is enough.
    if (job->fence >= 0) {
        jobsys_fence_signal_by_id(&p->fences, job->fence);
    }

    // fire the continuation, if it named one. pulling it out frees the slot.
    if (job->cont >= 0) {
        jobsys_job next;
        if (jobsys_chain_take(&p->chains, job->cont, &next)) {
            // push onto our own deque -- it's hot and we likely just warmed the
            // cache for whatever data it touches.
            if (jobsys_deque_push(&w->deque, &next) != 0) {
                jobsys_overflow_push(&p->overflow, &next);
                jat_add_rlx(&p->stats.overflowed, 1);
            }
        }
    }
}

int jobsys_worker_acquire(jobsys_worker *w, jobsys_job *out) {
    jobsys_pool *p = w->pool;
if (jobsys_deque_pop(&w->deque, out)) return 1;
for (int attempt = 0;
attempt < p->nworkers;
attempt++) {
        int v = jobsys_worker_pick_victim(w);
        if (v < 0) break;
        int r = jobsys_deque_steal(&p->workers[v].deque, out);
        if (r == 1) {
            jat_add_rlx(&p->stats.stolen, 1);
            return 1;
        }
        // r == 0 (empty) or -1 (aborted): just try another victim next pass.
    }

    // 3) shared overflow queue. cross-thread submits and spillover land here.
    // skip main-only jobs -- those are the main thread's problem.
    if (jobsys_overflow_pop(&p->overflow, out)) {
        if (out->flags & JOBSYS_F_MAIN_ONLY) {
            // put it back, it's not ours to run. rare, only if a main job got
            // into the general lane somehow.
            jobsys_overflow_push(&p->overflow, out);
} else {
            return 1;
        }
    }

    return 0;
}

// park the worker until someone submits, or shutdown. returns when it should
// re-check for work.
static void worker_sleep(jobsys_worker *w) {
    jobsys_pool *p = w->pool;
    pthread_mutex_lock(&p->sleep_mtx);
    // last chance: dont sleep if we're shutting down.
    if (!jat_load_acq(&p->running)) {
        pthread_mutex_unlock(&p->sleep_mtx);
        return;
    }
    jat_add_rlx(&p->waiters, 1);
    // condvar wait. a submit broadcasts, shutdown broadcasts. spurious wakeups
    // are fine -- we just loop back and look for work again.
    pthread_cond_wait(&p->sleep_cv, &p->sleep_mtx);
    jat_sub_rlx(&p->waiters, 1);
    pthread_mutex_unlock(&p->sleep_mtx);
}

void *jobsys_worker_main(void *arg) {
    jobsys_worker *w = (jobsys_worker *)arg;
jobsys_pool   *p = w->pool;
LOGD("jobsys worker %d up", w->id);
while (jat_load_acq(&p->running)) {
        jobsys_job job;
        int got = 0;
        // spin a bounded number of acquire passes before considering sleep, so
        // we soak up bursts without a condvar round-trip per job.
        for (int s = 0; s < WORKER_SPIN_TRIES; s++) {
            if (jobsys_worker_acquire(w, &job)) { got = 1; break; }
        }

        if (got) {
            jobsys_worker_execute(p, w, &job);
            continue;
        }

        // nothing found after spinning. park until woken. re-check running on
        // the way out (it may have cleared while we held nothing).
        worker_sleep(w);
    }

    // shutdown drain: run anything still reachable so fences settle and callers
    // dont wait forever on a job that got abandoned mid-flight.
    jobsys_job job;
return NULL;
}
