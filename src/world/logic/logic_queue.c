#include "logic_queue.h"
#include <stdint.h>

void logic_queue_init(logic_queue *q) {
    q->len = 0;
    q->seq = 0;
}

void logic_queue_clear(logic_queue *q) {
    q->len = 0;
    // leave seq alone; monotonic across clears is fine and avoids tie aliasing
}

int logic_queue_empty(const logic_queue *q) { return q->len == 0; }
int logic_queue_len(const logic_queue *q)   { return q->len; }

// an event a fires before b if it's due earlier, or same tick but inserted
// earlier. returns 1 if a should sit above b in the min-heap.
static int ev_less(const logic_event *a, const logic_event *b) {
    if (a->due_tick != b->due_tick) return a->due_tick < b->due_tick;
    return a->seq < b->seq;
}

static void sift_up(logic_queue *q, int i) {
    while (i > 0) {
        int parent = (i - 1) >> 1;
        if (!ev_less(&q->heap[i], &q->heap[parent])) break;
        logic_event tmp = q->heap[i];
        q->heap[i] = q->heap[parent];
        q->heap[parent] = tmp;
        i = parent;
    }
}

static void sift_down(logic_queue *q, int i) {
    for (;;) {
        int l = 2 * i + 1, r = 2 * i + 2, small = i;
        if (l < q->len && ev_less(&q->heap[l], &q->heap[small])) small = l;
        if (r < q->len && ev_less(&q->heap[r], &q->heap[small])) small = r;
        if (small == i) break;
        logic_event tmp = q->heap[i];
        q->heap[i] = q->heap[small];
        q->heap[small] = tmp;
        i = small;
    }
}

int logic_queue_push(logic_queue *q, uint64_t key, uint32_t now, uint32_t delay) {
    if (q->len >= LOGIC_QUEUE_CAP) return 0;
    logic_event e;
    e.key = key;
    e.due_tick = now + delay;
    e.seq = q->seq++;
    q->heap[q->len] = e;
    sift_up(q, q->len);
    q->len++;
    return 1;
}

uint32_t logic_queue_peek_tick(const logic_queue *q) {
    if (q->len == 0) return UINT32_MAX;
    return q->heap[0].due_tick;
}

int logic_queue_pop(logic_queue *q, logic_event *out) {
    if (q->len == 0) return 0;
    *out = q->heap[0];
    q->len--;
    if (q->len > 0) {
        q->heap[0] = q->heap[q->len];
        sift_down(q, 0);
    }
    return 1;
}
