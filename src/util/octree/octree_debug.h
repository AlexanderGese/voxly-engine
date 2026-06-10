#ifndef UTIL_OCTREE_DEBUG_H
#define UTIL_OCTREE_DEBUG_H

// introspection. stats for the f3-style overlay, a tree dump for when an insert
// goes sideways, and a wireframe-line extractor so the renderer can draw cell
// boundaries while debugging culling.

#include "octree.h"

typedef struct {
    int   node_count;       // live nodes
    int   leaf_count;
    int   item_count;
    int   max_depth;        // deepest occupied leaf
    int   empty_leaves;     // leaves holding nothing (split happened, drained)
    float avg_items;        // per non-empty leaf
    int   pool_cap;
    int   free_nodes;
} octree_stats;

void octree_collect_stats(const octree *t, octree_stats *out);

// dump the tree to the log, indented by depth. handy but spammy, gate it.
void octree_dump(const octree *t);

// write the 12 edges of every non-empty node's bounds into `lines` (a darray of
// vec3, two verts per edge so 24 vec3 per node). returns node count emitted.
int  octree_collect_lines(const octree *t, vec3 **lines);

#endif
