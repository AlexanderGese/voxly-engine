#include "jobsys_debug.h"

#include <stdio.h>

int64_t jobsys_debug_pending(const jobsys_pool *p) {
    int64_t total = 0;
    for (int i = 0; i < p->nworkers; i++) {
        total += jobsys_deque_size(&p->workers[i].deque);
    }
    // overflow len takes the queue lock; cast away const, it's a debug read and
    // the lock makes it safe regardless of our const promise.
    total += (int64_t)jobsys_overflow_len((jobsys_overflow *)&p->overflow);
    return total;
}

void jobsys_debug_dump(const jobsys_pool *p, strbuf *out) {
    jobsys_stats s = jobsys_get_stats(p);

    strbuf_appendf(out, "jobs: %llu run / %llu stolen / %llu spill / %llu main\n",
                   (unsigned long long)s.executed,
                   (unsigned long long)s.stolen,
                   (unsigned long long)s.overflowed,
                   (unsigned long long)s.main_ran);

    // submitted-vs-executed gap is roughly what's still outstanding. handy to
    // eyeball: if it climbs and never settles, a fence is leaking or a job
    // wedged. (asks the obvious question before you go spelunking with gdb.)
    unsigned long long inflight =
        (unsigned long long)(s.submitted > s.executed ? s.submitted - s.executed : 0);
    strbuf_appendf(out, "inflight ~%llu  workers %d\n", inflight, p->nworkers);

    strbuf_append(out, "deq : [");
    for (int i = 0; i < p->nworkers; i++) {
        strbuf_appendf(out, "%lld%s",
                       (long long)jobsys_deque_size(&p->workers[i].deque),
                       i + 1 < p->nworkers ? " " : "");
    }
    strbuf_appendf(out, "]  ovf %zu  parked %d\n",
                   jobsys_overflow_len((jobsys_overflow *)&p->overflow),
                   (int)jat_load_rlx((jat_i32 *)&p->waiters));
}

const char *jobsys_debug_oneline(const jobsys_pool *p, char *buf, int n) {
    if (n <= 0) return buf;
    int64_t pend = jobsys_debug_pending(p);
    int parked   = (int)jat_load_rlx((jat_i32 *)&p->waiters);
    // snprintf truncates and always null-terminates, which is what we want for a
    // hud string -- better a clipped line than a smashed stack.
    snprintf(buf, (size_t)n, "jobs %lld / w%d", (long long)pend,
             p->nworkers - parked);
    return buf;
}
