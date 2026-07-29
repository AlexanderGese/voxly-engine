#ifndef ENTITY_MOBS_MOB_PASSIVE_H
#define ENTITY_MOBS_MOB_PASSIVE_H

#include "mob_common.h"
#include "mob_rng.h"
#include "../../math/vec3.h"

// shared brain for passive animals (cow/pig/sheep/chicken). they wander
// aimlessly and bolt away from the player after taking a hit, calming down
// once far enough away. per-species files just call this with their kind.

void voxl_mob_passive_update(voxl_mob *m, voxl_mob_kind kind,
                             voxl_mob_rng *r, vec3 player_pos, float dt);

#endif
