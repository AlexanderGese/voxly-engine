#ifndef UTIL_OCTREE_H
#define UTIL_OCTREE_H

// sparse octree over a fixed world cube. items (id + aabb) get pushed as deep
// as they fully fit. leaves split when they overflow OCTREE_LEAF_CAP, and
// collapse back when a subtree drains. id->location map on the side so removal
// and moves dont need a full tree scan.
//
// usage is roughly:
// octree t; octree_init(&t, world_bounds);
// octree_insert(&t, id, box);
// ... octree_query_aabb(...) / octree_raycast(...) ...
// octree_remove(&t, id);  octree_free(&t);

#include "octree_types.h"
#include "octree_node.h"
#include "../hashmap.h"

typedef struct {
    octree_pool pool;
    int32_t     root;       // node index of the root, -1 before init
    aabb        bounds;     // total covered region
    hashmap     locate;     // id -> (node index packed in the val ptr), for O(1) remove
    int32_t     item_count;
} octree;

void octree_init(octree *t, aabb world_bounds);
void octree_free(octree *t);

// drop everything but keep the allocation around. cheap reset between frames if
// you rebuild the whole tree (some callers do, for moving entities).
void octree_clear(octree *t);

// add an item. if the id already exists it is replaced (treated as a move).
// returns 0 ok, -1 on oom or if box is fully outside world bounds.
int  octree_insert(octree *t, uint32_t id, uint32_t tag, aabb box);

// remove by id. returns 1 if something was removed, 0 if id was unknown.
int  octree_remove(octree *t, uint32_t id);

// convenience: remove + reinsert. faster than caller doing it because we can
// shortcut when the item didnt actually leave its current node.
int  octree_move(octree *t, uint32_t id, uint32_t tag, aabb new_box);

int  octree_count(const octree *t);

#endif
