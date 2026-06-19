#include "lightprop_queue.h"

void lp_queue_init(lp_queue *q) {
    q->head = 0;
    q->tail = 0;
    q->dropped = 0;
}

void lp_queue_reset(lp_queue *q) {
    // keep dropped around across resets within one operation so the driver can
    // log a single tally at the end.
    q->head = 0;
    q->tail = 0;
}

int lp_queue_empty(const lp_queue *q) {
    return q->head == q->tail;
}

int lp_queue_count(const lp_queue *q) {
    int n = q->tail - q->head;
    if (n < 0) n += LP_QCAP;
    return n;
}

int lp_queue_push(lp_queue *q, int x, int y, int z, uint8_t level) {
    int nt = q->tail + 1;
    if (nt >= LP_QCAP) nt = 0;
    if (nt == q->head) {
        q->dropped++;
        return 0;   // full
    }
    q->buf[q->tail].x = x;
    q->buf[q->tail].y = y;
    q->buf[q->tail].z = z;
    q->buf[q->tail].level = level;
    q->tail = nt;
    return 1;
}

int lp_queue_pop(lp_queue *q, lp_node *out) {
    if (q->head == q->tail) return 0;
    *out = q->buf[q->head];
    int nh = q->head + 1;
    if (nh >= LP_QCAP) nh = 0;
    q->head = nh;
    return 1;
}
