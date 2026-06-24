#ifndef WORLD_RIVERS_HEAP_H
#define WORLD_RIVERS_HEAP_H
#include "rivers_types.h"
typedef struct {
    int   cell;    // flat field index
    int   key;     // height we sort by (the fill height, not raw surface)
    int   seq;     // insertion order, used as a stable tie-break
} rivers_heap_node;
typedef struct {
    rivers_heap_node nodes[RIVERS_CELLS];
    int count;
    int seq;       // monotonically increasing insertion counter
} rivers_heap;
void rivers_heap_init(rivers_heap *h);
int  rivers_heap_empty(const rivers_heap *h);
void rivers_heap_push(rivers_heap *h, int cell, int key);
#endif
