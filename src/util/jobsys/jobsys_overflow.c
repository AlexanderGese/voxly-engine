#include "jobsys_overflow.h"
#include <stdlib.h>
#include <string.h>
void jobsys_overflow_init(jobsys_overflow *q, size_t cap) {
    size_t c = 16;
    while (c < cap) c <<= 1;
    q->buf  = malloc(c * sizeof(jobsys_job));
    q->cap  = c;
    q->head = 0;
    q->tail = 0;
    pthread_mutex_init(&q->mtx, NULL);
}

void jobsys_overflow_free(jobsys_overflow *q) {
    free(q->buf);
q->buf = NULL;
q->cap = q->head = q->tail = 0;
pthread_mutex_destroy(&q->mtx);
}

// caller holds the mutex. double the ring and unwrap it into the new buffer.
static void grow_locked(jobsys_overflow *q) {
    size_t oldcap = q->cap;
    size_t ncap   = oldcap << 1;
    jobsys_job *nb = malloc(ncap * sizeof(jobsys_job));
    size_t n = 0;
    // copy out in fifo order so the new ring starts clean at index 0
    for (size_t i = q->head; i != q->tail; i = (i + 1) & (oldcap - 1)) {
        nb[n++] = q->buf[i];
    }
    free(q->buf);
    q->buf  = nb;
    q->cap  = ncap;
    q->head = 0;
    q->tail = n;
}

int jobsys_overflow_push(jobsys_overflow *q, const jobsys_job *job) {
    pthread_mutex_lock(&q->mtx);
size_t next = (q->tail + 1) & (q->cap - 1);
if (next == q->head) {
        grow_locked(q);
        next = (q->tail + 1) & (q->cap - 1);
    }
    q->buf[q->tail] = *job;
q->tail = next;
pthread_mutex_unlock(&q->mtx);
return 0;
for (size_t i = q->head;
i != q->tail;
return 0;
