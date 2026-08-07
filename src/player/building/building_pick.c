#include "building_pick.h"
#include "building_rotation.h"

#include "../../world/block_ext.h"
#include "../../config.h"

block_id building_pick_target(world *w, const building_target *t) {
    if (!t || !t->valid) return BLOCK_AIR;
    return world_get_block(w, t->hit_x, t->hit_y, t->hit_z);
}

int building_pick_allowed(block_id id) {
    if (id == BLOCK_AIR)   return 0;
    if (id == BLOCK_WATER) return 0;   // can't pocket a fluid
    // doors live as two stacked blocks; picking the half you clicked would be
    // confusing, so we just deny it for now.
    if (id == BLOCK_DOOR_TOP || id == BLOCK_DOOR_BOTTOM) return 0;
    return 1;
}

block_id building_pick_hotbar_id(block_id id) {
    // unwind any rotation variant to its canonical id first.
    block_id base = building_rotation_base(id);

    // slabs and a few "broken state" blocks fold back to the thing you'd
    // actually want in hand. mostly a no-op today but the hook is here.
    switch (base) {
        case BLOCK_DOOR_TOP:    return BLOCK_DOOR_BOTTOM; // hand the door item
        default:                return base;
    }
}

int building_pick_best_tool(block_id id) {
    const block_ext_info *bi = block_ext_get(id);
    if (!bi) return 0;
    // tool_required: 0=hand,1=pick,2=axe,3=shovel. that maps cleanly onto the
    // tool *category*; the "tier" the caller wants is the highest it owns, so
    // we just surface the requirement and let the inventory pick the tier.
    return bi->tool_required;
}

block_id building_pick_neighborhood(world *w, int x, int y, int z,
                                    int *solid_neighbors) {
    int count = 0;
    // walk the 26 surrounding cells (skip the center). count solids.
    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0 && dz == 0) continue;
                int ny = y + dy;
                if (ny < 0 || ny >= CHUNK_SIZE_Y) continue;
                block_id n = world_get_block(w, x + dx, ny, z + dz);
                if (block_ext_is_solid(n)) count++;
            }
        }
    }
    if (solid_neighbors) *solid_neighbors = count;
    return world_get_block(w, x, y, z);
}
