#include "farming_plant.h"
#include "farming_block.h"
#include "farming_query.h"
#include "farming_def.h"
#include "../block.h"
#include <stddef.h>

int farming_plant_till(world *w, int wx, int wy, int wz) {
    block_id here = world_get_block(w, wx, wy, wz);
    if (here != BLOCK_DIRT && here != BLOCK_GRASS) return 0;
    // cant till if something sits on top; the hoe needs open sky-ish above.
    if (world_get_block(w, wx, wy + 1, wz) != BLOCK_AIR) return 0;

    world_set_block(w, wx, wy, wz, FARMING_BLOCK_FARMLAND);
    return 1;
}

int farming_plant_crop(world *w, int wx, int wy, int wz,
                       farming_crop_kind kind, uint32_t tick,
                       farming_crop *out) {
    if (kind <= FARMING_CROP_NONE || kind >= FARMING_CROP_COUNT) return 0;
    if (!farming_query_plantable(w, wx, wy, wz)) return 0;

    block_id cb = farming_block_for_crop(kind);
    if (cb == BLOCK_AIR) return 0;

    world_set_block(w, wx, wy, wz, cb);

    if (out) {
        out->wx = wx;
        out->wy = wy;
        out->wz = wz;
        out->kind = (uint8_t)kind;
        out->stage = 0;
        out->flags = FARMING_CROP_F_ALIVE;
        out->_pad = 0;
        out->growth_accum = 0.0f;
        out->planted_tick = tick;
    }
    return 1;
}

int farming_plant_uproot(world *w, int wx, int wy, int wz,
                         farming_crop_kind kind) {
    block_id here = world_get_block(w, wx, wy, wz);
    if (!farming_block_is_crop(here)) return 0;

    world_set_block(w, wx, wy, wz, BLOCK_AIR);

    // immature break drops: enough to replant, nothing more. matches the def's
    // seed identity so carrots/potatoes give themselves back.
    const farming_def *def = farming_def_get(kind);
    if (def->is_stem) return 1;            // one seed
    if (kind == FARMING_CROP_CARROT ||
        kind == FARMING_CROP_POTATO) return 1; // one tuber
    return 1;                              // wheat/beet: one seed
}
