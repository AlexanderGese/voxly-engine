#ifndef UTIL_BVH_TYPES_H
#define UTIL_BVH_TYPES_H

// shared types for the bvh. one header so build/refit/query/raycast dont have
// to include each other in a circle (same trick octree_types pulls).
//
// the bvh stores "primitives" which are just an id + a tag + an aabb. it has no
// idea what the id means - entity, chunk, collider, whatever - thats the
// callers problem. keeps the structure reusable, like every good accel struct
// should be.

#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include <stdint.h>

// max prims we let pile up in a leaf before the builder stops splitting. small
// leaves = deeper tree = more node hops; big leaves = more brute-force tests at
// the bottom. 4 tested out fine for entity-density scenes, dont overthink it.
#define BVH_LEAF_MAX_PRIMS    4

// hard cap on tree depth. a degenerate input (everything on one point) would
// otherwise recurse until the stack screams. if we hit this we just make a fat
// leaf and move on.
#define BVH_MAX_DEPTH         48

// number of buckets the SAH sweep bins primitives into per axis. 12 is the
// usual pbrt-ish number. more buckets = finer split candidate, diminishing
// returns past ~16.
#define BVH_SAH_BUCKETS       12

// cost the SAH model assigns to a ray-vs-node traversal step relative to a
// ray-vs-primitive test (which is 1.0). traversal is cheaper than a real prim
// intersection so we bias toward fewer, fatter nodes a touch.
#define BVH_SAH_TRAVERSAL_COST 0.125f

// one input primitive. id is opaque, tag is optional category bits (0 =
// untagged, matched against a query mask). box is world space bounds. we keep
// the full box, not a point, so big things dont get mis-binned.
typedef struct {
    uint32_t id;
    uint32_t tag;
    aabb     box;
} bvh_prim;

// one node in the flattened tree. this is a "linear bvh" layout: nodes live in
// a flat array, the left child is always the next node (idx+1), and we only
// store the right child / second-child offset. leaves carry a [first,count)
// slice into the reordered primitive array.
//
// count == 0 means interior node, use `second_child`. count > 0 means leaf, use
// `first_prim`. classic pbrt packing, keeps the node to 32 bytes-ish.
typedef struct {
    aabb     bounds;        // tight bounds over this subtree
    union {
        uint32_t first_prim;    // leaf: index of first prim in the ordered array
        uint32_t second_child;  // interior: index of the right child node
    } u;
    uint16_t count;         // leaf prim count, 0 if interior
    uint8_t  axis;          // interior: split axis (0/1/2), for ordered traversal
    uint8_t  _pad;
} bvh_node;

// result of a raycast. t is along the ray in dir units. hit_id is the prim id.
typedef struct {
    int      hit;
    float    t;
    uint32_t hit_id;
    uint32_t hit_tag;
    vec3     point;
} bvh_hit;

#endif
