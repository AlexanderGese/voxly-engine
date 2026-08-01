#include "projectile_sampler.h"

// we duplicate a *tiny* bit of block classification rather than depend on
// block_get() so the sampler seam can be exercised in a unit test without the
// block table being initialized. if these ever drift from block.h, the flush
// test in the demo will catch it (it asserts a known wall stops an arrow).

int projectile_block_is_solid(block_id id) {
    switch (id) {
        case BLOCK_AIR:
        case BLOCK_WATER:
            return 0;
        default:
            // everything else in the table collides. glass included on purpose:
            // arrows should clink off it, not phase through.
            return 1;
    }
}

int projectile_block_is_fluid(block_id id) {
    // only water for now. lava would slot in here when we add it, with a
    // separate "burns" flag the world loop can react to.
    return id == BLOCK_WATER;
}
