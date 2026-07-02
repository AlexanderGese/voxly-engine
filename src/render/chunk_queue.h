#ifndef RENDER_CHUNK_QUEUE_H
#define RENDER_CHUNK_QUEUE_H

#include "../world/chunk.h"

// priority queue for chunk meshing. chunks closer to the player get
// meshed first so pop-in is less noticeable.

#define CQ_MAX 256

typedef struct {
    chunk *c;
    float  dist_sq;
} cq_entry;

typedef struct {
    cq_entry entries[CQ_MAX];
    int      count;
} chunk_queue;

void chunk_queue_init(chunk_queue *q);
void chunk_queue_clear(chunk_queue *q);
void chunk_queue_push(chunk_queue *q, chunk *c, float dist_sq);
chunk *chunk_queue_pop(chunk_queue *q);  // returns closest
int  chunk_queue_empty(const chunk_queue *q);

#endif
