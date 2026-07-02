#include "chunk_queue.h"

void chunk_queue_init(chunk_queue *q) { q->count = 0; }
void chunk_queue_clear(chunk_queue *q) { q->count = 0; }

void chunk_queue_push(chunk_queue *q, chunk *c, float dist_sq) {
    if (q->count >= CQ_MAX) {
        // drop the farthest entry if full
        int farthest = 0;
        for (int i = 1; i < q->count; i++) {
            if (q->entries[i].dist_sq > q->entries[farthest].dist_sq)
                farthest = i;
        }
        if (dist_sq < q->entries[farthest].dist_sq) {
            q->entries[farthest].c = c;
            q->entries[farthest].dist_sq = dist_sq;
        }
        return;
    }
    q->entries[q->count].c = c;
    q->entries[q->count].dist_sq = dist_sq;
    q->count++;
}

chunk *chunk_queue_pop(chunk_queue *q) {
    if (q->count == 0) return 0;
    int closest = 0;
    for (int i = 1; i < q->count; i++) {
        if (q->entries[i].dist_sq < q->entries[closest].dist_sq)
            closest = i;
    }
    chunk *c = q->entries[closest].c;
    q->entries[closest] = q->entries[q->count - 1];
    q->count--;
    return c;
}

int chunk_queue_empty(const chunk_queue *q) { return q->count == 0; }
