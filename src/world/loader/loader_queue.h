#ifndef WORLD_LOADER_QUEUE_H
#define WORLD_LOADER_QUEUE_H

#include <stddef.h>
#include "loader_job.h"

// priority queue of jobs. binary min-heap ordered by job.priority (lower first),
// ties broken by serial then enqueue time so older work drains before newer at
// the same urgency -- otherwise a chunk on the ring edge could starve forever as
// closer chunks keep cutting the line. that actually happened, hence the tiebreak.

// grows on demand. i benchmarked a bucketed/calendar queue here once but the heap
// won on the chunk counts we deal with (a few thousand jobs tops) and it's less
// code to be wrong in.

typedef struct {
    loader_job *nodes;   // 1-based heap is tempting but we 0-base it
    size_t      len;
    size_t      cap;
} loader_queue;

void loader_queue_init(loader_queue *q, size_t initial_cap);
void loader_queue_free(loader_queue *q);

void loader_queue_push(loader_queue *q, loader_job job);

// pops the most urgent job into *out. returns 0 on success, -1 if empty.
int  loader_queue_pop(loader_queue *q, loader_job *out);

// peek without removing. returns 0/-1 like pop.
int  loader_queue_peek(const loader_queue *q, loader_job *out);

size_t loader_queue_len(const loader_queue *q);
int    loader_queue_empty(const loader_queue *q);

// drop every queued job targeting `key`. used when a chunk unloads so we dont
// pop work for a slot that's gone. linear scan + reheapify, called rarely.
// returns how many jobs were removed.
int  loader_queue_drop_key(loader_queue *q, uint64_t key);

// throw the whole thing away (world teardown / hard reload).
void loader_queue_clear(loader_queue *q);

#endif
