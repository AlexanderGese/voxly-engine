#ifndef UTIL_BVH_NODE_H
#define UTIL_BVH_NODE_H

// the flat node array + the ordered primitive array that back a built tree.
// nodes are stored depth-first so the left child of an interior node is always
// the very next slot - that's the whole point of the linear layout, it makes
// the common-case "descend left" a pointer bump with no indirection.

#include "bvh_types.h"

typedef struct {
    bvh_node *nodes;        // flat depth-first node array
    int32_t   node_count;   // live nodes
    int32_t   node_cap;     // allocated slots

    bvh_prim *prims;        // primitives in leaf order (reordered during build)
    int32_t   prim_count;
} bvh_storage;

// stand up empty storage with room for `prim_hint` primitives. the node array
// is sized to the worst case (2N-1) so the builder never reallocs mid-recursion
// and invalidates a node pointer it's holding.
void    bvh_storage_init(bvh_storage *s, int32_t prim_hint);
void    bvh_storage_free(bvh_storage *s);

// drop all nodes + prims but keep the allocations for a rebuild. cheap reset.
void    bvh_storage_clear(bvh_storage *s);

// append a blank node, returning its index. grows the array if the worst-case
// estimate was somehow too small (it shouldnt be, but belt and suspenders).
int32_t bvh_storage_push_node(bvh_storage *s);

// fast index -> node, no bounds check. callers trust their own indices.
static inline bvh_node *bvh_node_at(bvh_storage *s, int32_t idx) {
    return &s->nodes[idx];
}
static inline const bvh_node *bvh_node_at_c(const bvh_storage *s, int32_t idx) {
    return &s->nodes[idx];
}

// copy the caller's primitives in. the builder shuffles this internal copy, the
// caller's array is left untouched.
int     bvh_storage_set_prims(bvh_storage *s, const bvh_prim *src, int32_t n);

#endif
