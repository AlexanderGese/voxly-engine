#ifndef ENTITY_MOBS_MOB_ZOMBIE_H
#define ENTITY_MOBS_MOB_ZOMBIE_H

#include "mob_common.h"
#include "mob_rng.h"
#include "../../math/vec3.h"

// zombie: slow shambling melee hostile. wanders until it sees the player,
// then chases and bashes in melee range. give up if the player gets away.

// returns the damage the zombie deals this frame (0 if no swing landed).
int voxl_mob_zombie_update(voxl_mob *m, voxl_mob_rng *r,
                           vec3 player_pos, float dt);

#endif
