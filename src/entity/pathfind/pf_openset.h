#ifndef ENTITY_PATHFIND_PF_OPENSET_H
#define ENTITY_PATHFIND_PF_OPENSET_H

#include "pf_nodepool.h"

// binary min-heap of open nodes keyed on f. the old astar.c does a linear
// scan for the lowest f every iteration which is O(n^2); on a 49x49 window
// that hurts, so this one keeps a proper heap.
//
// the heap stores node *indices* into the pool. each node tracks its own
// heap_pos so decrease-key is O(log n) instead of a search.

typedef struct {
    int          slots[PF_MAX_NODES];
    int          len;
    pf_nodepool *pool;          // borrowed, not owned
} pf_openset;

void pf_openset_init(pf_openset *h, pf_nodepool *pool);
int  pf_openset_empty(const pf_openset *h);

// push node idx, or re-sift it if its f dropped (decrease-key). safe to
// call on a node already in the heap.
void pf_openset_push(pf_openset *h, int idx);

// pop the node with the smallest f. returns its pool index, or -1 if empty.
int  pf_openset_pop(pf_openset *h);

#endif
