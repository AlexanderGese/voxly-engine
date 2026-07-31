#ifndef ENTITY_PATHFIND_PF_NODE_H
#define ENTITY_PATHFIND_PF_NODE_H

#include "pf_types.h"

// a search node. parent is an index into the pool (-1 = root). costs are
// fixed-point: we scale by 10 so a diagonal (14) vs cardinal (10) stays
// integer and we never touch floats in the hot loop.

#define PF_COST_CARD   10
#define PF_COST_DIAG   14
#define PF_COST_STEP    4   // small surcharge for changing height

typedef struct {
    pf_coord c;
    int32_t  g;        // cost from start
    int32_t  f;        // g + h
    int32_t  parent;   // index in pool, -1 for the start node
    int32_t  heap_pos; // index in the open heap, -1 if not / no longer open
    uint8_t  closed;
} pf_node;

#endif
