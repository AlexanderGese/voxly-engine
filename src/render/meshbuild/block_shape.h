#ifndef RENDER_MESHBUILD_BLOCK_SHAPE_H
#define RENDER_MESHBUILD_BLOCK_SHAPE_H

// not every block is a full 1x1x1 cube. greedy meshing and face culling both
// assume full cubes — if you let a torch or a (future) slab merge into a stone
// run youll get a stretched torch sprite across half a wall, which is exactly
// the bug that made me write this file.
//
// so: classify each block's mesh shape. cubes go through greedy. non-cubes get
// emitted individually (and crosses get a different geometry entirely). the
// builder consults this before ever touching the greedy mask.

#include "../../world/block.h"

typedef enum {
    MB_SHAPE_CUBE = 0,    // full opaque/transparent box, greedy-able
    MB_SHAPE_CROSS,       // two diagonal billboard quads (torch-ish, plants)
    MB_SHAPE_NONE,        // emits no geometry (air)
} mb_shape;

mb_shape mb_block_shape(block_id id);

// convenience predicates the builder reads constantly.
int  mb_shape_is_cube(block_id id);
int  mb_shape_is_cross(block_id id);

// a block only participates in greedy face culling against its neighbours when
// both sides are cubes. a cross next to stone must NOT cull the stone face.
int  mb_shape_culls(block_id id);

#endif
