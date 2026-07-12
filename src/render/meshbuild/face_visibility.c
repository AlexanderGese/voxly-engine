#include "face_visibility.h"
#include "block_shape.h"

int mb_face_renderable(block_id here) {
    if (block_is_air(here)) return 0;
    // cross/billboard blocks (torch, future plants) dont emit box faces — they
    // have their own geometry path, so the greedy cube mesher skips them here.
    if (!mb_shape_is_cube(here)) return 0;
    return 1;
}

int mb_face_visible(block_id here, block_id there) {
    if (!mb_face_renderable(here)) return 0;

    // opaque neighbour hides us completely.
    if (block_is_opaque(there)) return 0;

    // same transparent material butted together: drop the shared interior wall.
    // block_is_opaque(here) being false implies here is transparent here.
    if (!block_is_opaque(here) && there == here) return 0;

    return 1;
}

int mb_face_mergeable(block_id a, int tile_a, block_id b, int tile_b) {
    // must be the exact same surface. block id alone isnt enough because a
    // block can use different tiles per face, but within one greedy pass the
    // face is fixed, so tile is what actually varies — check both anyway, its
    // cheap and it documents intent.
    if (a != b) return 0;
    if (tile_a != tile_b) return 0;
    return 1;
}
