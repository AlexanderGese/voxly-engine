#ifndef WORLD_FARMING_GROWTH_H
#define WORLD_FARMING_GROWTH_H

#include "../world.h"
#include "farming_types.h"
#include "farming_rng.h"
#include "farming_def.h"

// crop growth. a crop accumulates growth points each growth tick. the per-tick
// gain is base_rate scaled by hydration, light, and a small per-tile/per-crop
// neighbor bonus (rows of the same crop grow faster, the "farmland row" rule).
// crossing points_per_stage bumps the stage; hitting max_stage marks mature.

// the environment a single growth tick sees. assembled by the field driver from
// the tile + a couple of world probes so growth itself stays world-agnostic and
// testable.
typedef struct {
    float hydration;   // 0..1 from farming_hydration_factor
    int   light;       // 0..MAX_LIGHT
    float neighbor;    // 0..1 same-crop-neighbor bonus
    int   supported;   // farmland still under it?
} farming_env;

// compute the same-crop row bonus for a crop at (wx,wy,wz). looks at the four
// cardinal + four diagonal neighbors and rewards orderly rows over a scattered
// patch. returns 0..1.
float farming_growth_neighbor_bonus(world *w, int wx, int wy, int wz,
                                    farming_crop_kind kind);

// advance one growth tick. mutates crop->stage / growth_accum / flags. `seed`
// salts the per-tick grow roll so it stays deterministic per (tile, tick).
// returns 1 if the crop advanced a stage this tick.
int farming_growth_tick(farming_crop *crop, const farming_def *def,
                        const farming_env *env, uint32_t seed);

// has the crop reached its mature stage?
int farming_growth_is_mature(const farming_crop *crop, const farming_def *def);

#endif
