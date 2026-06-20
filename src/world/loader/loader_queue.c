#include "loader_queue.h"

#include <stdlib.h>
#include <string.h>
#include "../../util/log.h"

// strict weak ordering: returns nonzero if a should sit ABOVE b in the heap
// (i.e. comes out first). priority dominates, then serial (older = smaller),
// then enqueue time. all three so the order is total and the heap is stable-ish.
static int job_before(const loader_job *a, const loader_job *b) {
    if (a->priority != b->priority) return a->priority < b->priority;
    if (a->serial   != b->serial)   return a->serial   < b->serial;
    return a->enqueued_us < b->enqueued_us;
}

static void heap_swap(loader_queue *q, size_t i, size_t j) {
    loader_job t = q->nodes[i];
    q->nodes[i] = q->nodes[j];
    q->nodes[j] = t;
}

static void sift_up(loader_queue *q, size_t i) {
    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (!job_before(&q->nodes[i], &q->nodes[parent])) break;
        heap_swap(q, i, parent);
        i = parent;
    }
}

static void sift_down(loader_queue *q, size_t i) {
    for (;;) {
        size_t l = 2 * i + 1;
        size_t r = 2 * i + 2;
        size_t best = i;
        if (l < q->len && job_before(&q->nodes[l], &q->nodes[best])) best = l;
        if (r < q->len && job_before(&q->nodes[r], &q->nodes[best])) best = r;
        if (best == i) break;
        heap_swap(q, i, best);
        i = best;
    }
}

static void ensure_cap(loader_queue *q, size_t need) {
    if (need <= q->cap) return;
    size_t newcap = q->cap ? q->cap * 2 : 64;
    while (newcap < need) newcap *= 2;
    loader_job *n = realloc(q->nodes, newcap * sizeof *n);
    if (!n) {
        LOGE("loader_queue: realloc to %zu failed", newcap);
        return;     // push will silently drop. it'll get re-enqueued next frame.
    }
    q->nodes = n;
    q->cap   = newcap;
}

void loader_queue_init(loader_queue *q, size_t initial_cap) {
    q->nodes = NULL;
    q->len   = 0;
    q->cap   = 0;
    if (initial_cap) ensure_cap(q, initial_cap);
}

void loader_queue_free(loader_queue *q) {
    free(q->nodes);
    q->nodes = NULL;
    q->len = q->cap = 0;
}

void loader_queue_push(loader_queue *q, loader_job job) {
    ensure_cap(q, q->len + 1);
    if (q->len >= q->cap) return;   // alloc failed upstream, drop quietly
    q->nodes[q->len] = job;
    sift_up(q, q->len);
    q->len++;
}

int loader_queue_pop(loader_queue *q, loader_job *out) {
    if (q->len == 0) return -1;
    *out = q->nodes[0];
    q->len--;
    if (q->len > 0) {
        q->nodes[0] = q->nodes[q->len];
        sift_down(q, 0);
    }
    return 0;
}

int loader_queue_peek(const loader_queue *q, loader_job *out) {
    if (q->len == 0) return -1;
    *out = q->nodes[0];
    return 0;
}

size_t loader_queue_len(const loader_queue *q) { return q->len; }
int    loader_queue_empty(const loader_queue *q) { return q->len == 0; }

int loader_queue_drop_key(loader_queue *q, uint64_t key) {
    // compact in place then rebuild the heap. O(n) but key drops are rare and the
    // alternative (sift each hole) is fiddly to get right with duplicate keys.
    size_t w = 0;
    int dropped = 0;
    for (size_t r = 0; r < q->len; r++) {
        if (q->nodes[r].key == key) { dropped++; continue; }
        q->nodes[w++] = q->nodes[r];
    }
    q->len = w;
    // heapify from the last internal node down.
    if (q->len > 1) {
        for (size_t i = q->len / 2; i-- > 0; ) sift_down(q, i);
    }
    return dropped;
}

void loader_queue_clear(loader_queue *q) {
    q->len = 0;
}
