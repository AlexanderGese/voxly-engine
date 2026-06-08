#ifndef UTIL_BVH_DEBUG_H
#define UTIL_BVH_DEBUG_H

// introspection. stats for the f3-style overlay, a tree dump for when a build
// goes sideways, and a wireframe-line extractor so the renderer can draw node
// boxes while you stare at why the traversal isnt pruning what you expected.

#include "bvh.h"

typedef struct {
    int   node_count;       // total nodes
    int   leaf_count;
    int   interior_count;
    int   prim_count;
    int   max_depth;        // deepest leaf
    int   min_leaf_prims;   // smallest / largest leaf occupancy seen
    int   max_leaf_prims;
    float avg_leaf_prims;   // per leaf
    float sah_cost;         // the build's recorded cost proxy
    float root_area;        // surface area of the root bounds
} bvh_stats;

void bvh_collect_stats(const bvh *b, bvh_stats *out);

// dump the tree to the log, indented by depth. spammy on a big tree, gate it
// behind a keybind. shows leaf/interior, bounds, prim slices.
void bvh_dump(const bvh *b);

// write the 12 edges of every node's bounds into `lines` (a darray of vec3, two
// verts per edge => 24 vec3 per node). returns the number of nodes emitted.
// pass leaves_only to skip interior boxes and just draw the leaf cells.
int  bvh_collect_lines(const bvh *b, vec3 **lines, int leaves_only);

#endif
