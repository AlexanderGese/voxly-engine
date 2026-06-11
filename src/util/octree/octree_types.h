#ifndef UTIL_OCTREE_TYPES_H
#define UTIL_OCTREE_TYPES_H

// shared types for the sparse octree + spatial hash. kept in one place so the
// query/raycast/shash files dont have to include each other in a circle.
//
// the octree stores "items" which are just an id + an aabb. it doesnt care
// what the id means (entity, region, light emitter, whatever) - thats the
// callers problem. keeps this thing reusable.

#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include <stdint.h>

// max depth. world is 128 tall so an 8-deep tree over a 512 cube cell gets us
// down to 2-unit leaves which is plenty for entity broadphase. dont go nuts,
// each level doubles the node count in the worst case.
#define OCTREE_MAX_DEPTH      8

// how many items we let pile up in a leaf before trying to split it. classic
// loose-octree-ish threshold. measured this once, 8 was the sweet spot.
#define OCTREE_LEAF_CAP       8

// node child indexing. bit 0 = x, bit 1 = y, bit 2 = z. dont reorder these,
// the morton code and the child-pick math both assume this layout.
#define OCTREE_OCT_NX_NY_NZ   0
#define OCTREE_OCT_PX_NY_NZ   1
#define OCTREE_OCT_NX_PY_NZ   2
#define OCTREE_OCT_PX_PY_NZ   3
#define OCTREE_OCT_NX_NY_PZ   4
#define OCTREE_OCT_PX_NY_PZ   5
#define OCTREE_OCT_NX_PY_PZ   6
#define OCTREE_OCT_PX_PY_PZ   7

// an opaque-ish payload. id is whatever the caller wants. box is the world
// space bounds. we keep the box (not just a point) so big things straddle
// fewer cells and queries stay honest.
typedef struct {
    uint32_t id;
    uint32_t tag;     // optional category bits, caller defined, 0 = untagged
    aabb     box;
} octree_item;

// one node. children are indices into the node pool, not pointers, so the pool
// can realloc without us chasing dangling refs. -1 means no child.
// items[] is a small dynarray (darray.h) of octree_item, only non-null on
// leaves that actually hold something.
typedef struct {
    aabb          bounds;     // this node's region of space
    int32_t       child[8];   // pool indices, -1 if absent
    octree_item  *items;      // darray, may be NULL
    uint16_t      count;      // items in this node + whole subtree, for culling
    uint8_t       depth;
    uint8_t       leaf;       // 1 if no children
} octree_node;

// result of a raycast. t is along the ray in dir units. hit_id is the item id.
typedef struct {
    int      hit;
    float    t;
    uint32_t hit_id;
    uint32_t hit_tag;
    vec3     point;
} octree_hit;

#endif
