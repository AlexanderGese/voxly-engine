#include "farming_tile.h"
#include "farming_block.h"
#include "../block.h"

void farming_tile_init(farming_tile *t, int wx, int wy, int wz) {
    t->wx = wx;
    t->wy = wy;
    t->wz = wz;
    // fresh till starts at half wet; feels right, and stops crops insta-stalling
    // if you plant before watering.
    t->hydration = FARMING_HYDRATION_MAX / 2;
    t->has_crop = 0;
    t->trample = 0;
    t->dry_timer = 0.0f;
}

int farming_tile_trample(farming_tile *t) {
    if (t->trample < 255) t->trample++;
    return t->trample >= FARMING_TRAMPLE_CAP;
}

farming_tile_outcome farming_tile_decay(const farming_tile *t, int has_crop,
                                        float dt) {
    (void)dt; // hydration_step already advanced dry_timer; we only read it.

    // trampled past the cap: gone regardless of moisture.
    if (t->trample >= FARMING_TRAMPLE_CAP) return FARMING_TILE_REVERT;

    // occupied farmland never reverts from dryness alone; crops hold the soil.
    if (has_crop) return FARMING_TILE_OK;

    // empty + bone dry + has sat that way long enough -> back to dirt.
    if (t->hydration == 0 && t->dry_timer >= FARMING_REVERT_SECONDS)
        return FARMING_TILE_REVERT;

    return FARMING_TILE_OK;
}

block_id farming_tile_revert_world(world *w, const farming_tile *t) {
    block_id removed = BLOCK_AIR;

    block_id above = world_get_block(w, t->wx, t->wy + 1, t->wz);
    if (farming_block_is_crop(above)) {
        removed = above;
        world_set_block(w, t->wx, t->wy + 1, t->wz, BLOCK_AIR);
    }

    // only knock farmland back; if something already replaced it, leave it be.
    if (farming_block_is_farmland(world_get_block(w, t->wx, t->wy, t->wz)))
        world_set_block(w, t->wx, t->wy, t->wz, BLOCK_DIRT);

    return removed;
}
