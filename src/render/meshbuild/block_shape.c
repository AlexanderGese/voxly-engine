#include "block_shape.h"

mb_shape mb_block_shape(block_id id) {
    if (block_is_air(id)) return MB_SHAPE_NONE;

    switch (id) {
    case BLOCK_TORCH:
        // the one non-cube we ship right now. rendered as a thin cross so it
        // reads from every angle. doesnt tile, doesnt cull, doesnt greedy.
        return MB_SHAPE_CROSS;
    default:
        return MB_SHAPE_CUBE;
    }
}

int mb_shape_is_cube(block_id id) {
    return mb_block_shape(id) == MB_SHAPE_CUBE;
}

int mb_shape_is_cross(block_id id) {
    return mb_block_shape(id) == MB_SHAPE_CROSS;
}

int mb_shape_culls(block_id id) {
    // only full cubes hide neighbour faces. air and crosses let everything
    // around them stay visible.
    return mb_shape_is_cube(id);
}
