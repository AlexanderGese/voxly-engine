#ifndef UTIL_BVH_H
#define UTIL_BVH_H

// bounding volume hierarchy. you hand it a flat list of primitives (id + tag +
// aabb) and it builds a binary tree with a surface-area-heuristic split, stored
// flat for cache-friendly traversal. then you ray it, box-query it, or - if the
// prims only moved a little - refit it in place instead of rebuilding.
//
// it does NOT own a moving set the way the octree does (no insert/remove). a
// bvh's strength is a static-ish batch you query a lot; for that you rebuild or
// refit per frame. usage:
//
// bvh b; bvh_init(&b);
// bvh_build(&b, prims, n);            // SAH build
// bvh_hit h = bvh_raycast(&b, o, d, PLAYER_REACH);
// ...move prims a bit, then either...
// bvh_refit(&b);                      // cheap, keeps topology
// bvh_build(&b, prims, n);            // full rebuild when it got too loose
// bvh_free(&b);

#include "bvh_types.h"
#include "bvh_node.h"

typedef struct {
    bvh_storage store;
    int32_t     root;       // node index of the root, -1 if empty
    int         built;      // 1 once a successful build has happened
    float       last_cost;  // SAH cost estimate of the last build, for tuning
} bvh;

void bvh_init(bvh *b);
void bvh_free(bvh *b);

// drop the tree but keep buffers around for the next build. cheap.
void bvh_clear(bvh *b);

int  bvh_prim_count(const bvh *b);
int  bvh_node_count(const bvh *b);
int  bvh_is_built(const bvh *b);

// world bounds of the whole tree (root node bounds). empty box if not built.
aabb bvh_bounds(const bvh *b);

#endif
