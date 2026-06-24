#ifndef WORLD_RIVERS_HEAP_H
#define WORLD_RIVERS_HEAP_H

#include "rivers_types.h"

// a tiny binary min-heap keyed on cell height. the priority-flood lake pass
// pulls the lowest unprocessed boundary cell off this repeatedly, which is the
// whole trick that makes barnes/lehman depression filling run in n log n
// instead of iterating-until-stable forever.
//
// it stores cell indices and a key per entry. fixed capacity at RIVERS_CELLS
// because a cell only ever gets pushed once (we mark it the moment it lands on
// the queue), so the heap can never need more slots than there are cells.

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

// push a cell with a sort key. ties break toward whatever was inserted first,
// which keeps the flood deterministic regardless of scan order.
void rivers_heap_push(rivers_heap *h, int cell, int key);

// pop the lowest cell. returns its flat index and writes the key back out.
// undefined if the heap is empty, so check rivers_heap_empty first.
int  rivers_heap_pop(rivers_heap *h, int *out_key);

#endif
