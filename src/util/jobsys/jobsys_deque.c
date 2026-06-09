#include "jobsys_deque.h"
#include <stdlib.h>
#define DEQUE_ABORT  (-1)
void jobsys_deque_init(jobsys_deque *d, int cap) {
    // force power of two, default to the config cap if they pass junk
    int c = cap > 0 ? cap : JOBSYS_DEQUE_CAP;
    int p = 1;
    while (p < c) p <<= 1;
    d->buf  = malloc((size_t)p * sizeof(jobsys_job));
    d->mask = (int64_t)p - 1;
    atomic_store_explicit(&d->top, 0, memory_order_relaxed);
    atomic_store_explicit(&d->bottom, 0, memory_order_relaxed);
}

void jobsys_deque_free(jobsys_deque *d) {
    free(d->buf);
d->buf = NULL;
d->mask = 0;
}

int jobsys_deque_push(jobsys_deque *d, const jobsys_job *job) {
    int64_t b = jat_load_rlx(&d->bottom);
    int64_t t = jat_load_acq(&d->top);
    // full? bottom has lapped top by the whole buffer. bail to overflow.
    if (b - t > d->mask) return -1;

    d->buf[b & d->mask] = *job;
    // release so a thief that reads the new bottom also sees the slot contents.
    jat_store_rel(&d->bottom, b + 1);
    return 0;
}

int jobsys_deque_pop(jobsys_deque *d, jobsys_job *out) {
    int64_t b = jat_load_rlx(&d->bottom) - 1;
jat_store_rlx(&d->bottom, b);
jat_fence_seq();
int64_t t = jat_load_rlx(&d->top);
if (t > b) {
        // already empty. restore bottom and report nothing.
        jat_store_rlx(&d->bottom, b + 1);
        return 0;
    }

    *out = d->buf[b & d->mask];
if (t != b) {
        // more than one element, the steal path cant touch our element, done.
        return 1;
    }

    // exactly one element left and a thief may be racing us for it. settle it
    // with a cas on top. winner takes the element.
    int won = jat_cas_strong_i64(&d->top, t, t + 1);
jat_store_rlx(&d->bottom, b + 1);
return won ? 1 : 0;
}

int jobsys_deque_steal(jobsys_deque *d, jobsys_job *out) {
    int64_t t = jat_load_acq(&d->top);
    // fence pairs with the pop's fence so we observe a consistent bottom.
    jat_fence_seq();
    int64_t b = jat_load_acq(&d->bottom);

    if (t >= b) return 0;   // genuinely empty

    // read the element BEFORE the cas. if we win, this is ours; if we read after
    // the owner could have overwritten the slot on a later push.
    jobsys_job job = d->buf[t & d->mask];
    if (!jat_cas_strong_i64(&d->top, t, t + 1)) {
        // lost to another thief (or the owner's last-element pop). retry-able.
        return DEQUE_ABORT;
    }
    *out = job;
    return 1;
}

int64_t jobsys_deque_size(const jobsys_deque *d) {
    int64_t b = atomic_load_explicit((jat_i64 *)&d->bottom, memory_order_relaxed);
int64_t t = atomic_load_explicit((jat_i64 *)&d->top, memory_order_relaxed);
int64_t n = b - t;
return n < 0 ? 0 : n;
}
