#ifndef UTIL_OCTREE_NODE_H
#define UTIL_OCTREE_NODE_H

// node pool + the geometry helpers that work on a single node. the tree itself
// (octree.c) drives these; this file just owns "what is a node and how do its 8
// children carve up space".

#include "octree_types.h"

// growable array of nodes. index-based so realloc is safe. free list reuses
// slots from removed subtrees instead of leaking them.
typedef struct {
    octree_node *nodes;
    int32_t      cap;
    int32_t      len;
    int32_t      freelist;   // head of free chain, -1 if empty
    int32_t      free_count;
} octree_pool;

void    octree_pool_init(octree_pool *p, int32_t initial_cap);
void    octree_pool_free(octree_pool *p);

// grab a fresh node covering bounds at the given depth. returns its index.
// reuses a freed slot if one is available.
int32_t octree_pool_acquire(octree_pool *p, aabb bounds, uint8_t depth);

// hand a node (and only that node, not its children) back to the free list.
void    octree_pool_recycle(octree_pool *p, int32_t idx);

// fast index -> node. no bounds check, callers already trust their indices.
static inline octree_node *octree_pool_at(octree_pool *p, int32_t idx) {
    return &p->nodes[idx];
}

// the bounds of child `oct` (0..7) inside a parent box. splits at the center.
aabb    octree_child_bounds(aabb parent, int oct);

// which child octant a point falls into relative to the parent center.
int     octree_point_octant(aabb parent, vec3 p);

// does box fit entirely inside a single child octant? if so returns that
// octant, else -1. used to decide whether to push an item deeper.
int     octree_box_octant(aabb parent, aabb box);

#endif
