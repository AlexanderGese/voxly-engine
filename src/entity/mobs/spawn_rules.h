#ifndef ENTITY_MOBS_SPAWN_RULES_H
#define ENTITY_MOBS_SPAWN_RULES_H

#include "mob_common.h"
#include "mob_rng.h"
#include "../../math/vec3.h"

// decides what / whether to spawn given crude environment hints. no world
// access; the caller feeds in light level, surface block solidity, etc.

// info about a candidate spawn spot.
typedef struct {
    vec3 pos;
    int  light;         // 0..15, 0 = pitch black
    int  on_solid;      // 1 if there's solid ground under pos
    int  in_water;      // 1 if the column is water
    int  sky_visible;   // 1 if open to the sky (surface)
} voxl_spawn_ctx;

// pick a mob kind for this spot, or VOXL_MOB_NONE if nothing should spawn.
// hostiles want darkness, passives want grassy daylight surface.
voxl_mob_kind voxl_spawn_pick(const voxl_spawn_ctx *ctx, voxl_mob_rng *r);

// would `kind` survive a spawn at this spot? (light/space sanity check)
int voxl_spawn_is_valid(voxl_mob_kind kind, const voxl_spawn_ctx *ctx);

// soft cap helper: returns 1 if we're under the per-kind budget.
int voxl_spawn_under_cap(voxl_mob_kind kind, int current_of_kind);

#endif
