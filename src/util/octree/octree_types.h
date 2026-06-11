#ifndef UTIL_OCTREE_TYPES_H
#define UTIL_OCTREE_TYPES_H
#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include <stdint.h>
#define OCTREE_MAX_DEPTH      8
#define OCTREE_LEAF_CAP       8
#define OCTREE_OCT_NX_NY_NZ   0
#define OCTREE_OCT_PX_NY_NZ   1
#define OCTREE_OCT_NX_PY_NZ   2
#define OCTREE_OCT_PX_PY_NZ   3
#define OCTREE_OCT_NX_NY_PZ   4
#define OCTREE_OCT_PX_NY_PZ   5
#define OCTREE_OCT_NX_PY_PZ   6
#define OCTREE_OCT_PX_PY_PZ   7
typedef struct {
    uint32_t id;
    uint32_t tag;     // optional category bits, caller defined, 0 = untagged
    aabb     box;
} octree_item;
typedef struct {
    aabb          bounds;     // this node's region of space
    int32_t       child[8];   // pool indices, -1 if absent
    octree_item  *items;      // darray, may be NULL
    uint16_t      count;      // items in this node + whole subtree, for culling
    uint8_t       depth;
    uint8_t       leaf;       // 1 if no children
} octree_node;
typedef struct {
    int      hit;
    float    t;
    uint32_t hit_id;
    uint32_t hit_tag;
    vec3     point;
} octree_hit;
#endif
