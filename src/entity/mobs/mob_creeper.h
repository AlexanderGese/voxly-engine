#ifndef ENTITY_MOBS_MOB_CREEPER_H
#define ENTITY_MOBS_MOB_CREEPER_H

#include "mob_common.h"
#include "mob_rng.h"
#include "../../math/vec3.h"

// creeper: silent stalker. chases the player, and when close it lights a
// fuse; if the player stays near, it detonates. backing away cancels it.

// result of one update tick.
typedef struct {
    int   exploded;     // 1 if the creeper blew up this frame
    float fuse;         // current fuse charge 0..1 (for the swell animation)
    float blast_radius; // populated when exploded, else 0
} voxl_mob_creeper_result;

voxl_mob_creeper_result
voxl_mob_creeper_update(voxl_mob *m, voxl_mob_rng *r,
                        vec3 player_pos, float dt);

#endif
