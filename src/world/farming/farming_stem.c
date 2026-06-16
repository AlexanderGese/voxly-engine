#include "farming_stem.h"
#include "farming_block.h"
#include "farming_query.h"
#include "farming_def.h"
#include "farming_growth.h"
#include "../block.h"
#include <stddef.h>

int farming_stem_try_fruit(world *w, farming_crop *stem, farming_rng *rng) {
    farming_crop_kind kind = (farming_crop_kind)stem->kind;
    const farming_def *def = farming_def_get(kind);
    if (!def->is_stem) return 0;
    if (!farming_growth_is_mature(stem, def)) return 0;
    if (stem->flags & FARMING_CROP_F_FRUITED) return 0; // already has a fruit

    if (!farming_rng_chance(rng, FARMING_STEM_FRUIT_CHANCE)) return 0;

    int dx[4], dz[4];
    int n = farming_query_free_fruit_spots(w, stem->wx, stem->wy, stem->wz,
                                           dx, dz);
    if (n == 0) return 0; // hemmed in; try again next tick

    // pick one of the free spots uniformly.
    int pick = (int)(farming_rng_next(rng) % (uint32_t)n);
    block_id fruit = farming_fruit_block(kind);
    if (fruit == 0) return 0;

    int fx = stem->wx + dx[pick];
    int fz = stem->wz + dz[pick];
    world_set_block(w, fx, stem->wy, fz, fruit);
    stem->flags |= FARMING_CROP_F_FRUITED;
    return 1;
}

int farming_stem_recheck(world *w, farming_crop *stem) {
    if (!(stem->flags & FARMING_CROP_F_FRUITED)) return 0;

    farming_crop_kind kind = (farming_crop_kind)stem->kind;
    block_id fruit = farming_fruit_block(kind);
    if (fruit == 0) { stem->flags &= ~FARMING_CROP_F_FRUITED; return 0; }

    // scan the four cardinals; if our fruit is still adjacent, nothing changed.
    static const int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    for (int i = 0; i < 4; i++) {
        block_id b = world_get_block(w, stem->wx + dirs[i][0], stem->wy,
                                     stem->wz + dirs[i][1]);
        if (b == fruit) return 0; // fruit intact, still idle
    }

    // fruit gone -> clear the flag, stem is free to fruit again.
    stem->flags &= ~FARMING_CROP_F_FRUITED;
    return 1;
}
