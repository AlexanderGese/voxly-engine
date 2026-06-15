#include "colorlight_queue.h"

// two static rings. add flood eats one, remove flood eats both (its own work
// list plus a re-light list which we route through the add ring).
static colorlight_qnode s_add_buf[COLORLIGHT_QMAX];
static colorlight_qnode s_rem_buf[COLORLIGHT_QMAX];

static colorlight_queue s_add = { s_add_buf, 0, 0, COLORLIGHT_QMAX };
static colorlight_queue s_rem = { s_rem_buf, 0, 0, COLORLIGHT_QMAX };

colorlight_queue *colorlight_queue_add(void)    { return &s_add; }
colorlight_queue *colorlight_queue_remove(void) { return &s_rem; }

void colorlight_queue_reset(colorlight_queue *q) {
    q->head = q->tail = 0;
}

int colorlight_queue_empty(const colorlight_queue *q) {
    return q->head == q->tail;
}

int colorlight_queue_count(const colorlight_queue *q) {
    int n = q->tail - q->head;
    if (n < 0) n += q->cap;
    return n;
}

void colorlight_queue_push(colorlight_queue *q, int x, int y, int z, uint8_t chan, uint8_t level) {
    int nt = (q->tail + 1) % q->cap;
    if (nt == q->head) return; // full, drop it. relight pass will heal gaps.
    colorlight_qnode *n = &q->buf[q->tail];
    n->x = x; n->y = y; n->z = z;
    n->chan = chan; n->level = level;
    q->tail = nt;
}

int colorlight_queue_pop(colorlight_queue *q, colorlight_qnode *out) {
    if (colorlight_queue_empty(q)) return 0;
    *out = q->buf[q->head];
    q->head = (q->head + 1) % q->cap;
    return 1;
}
