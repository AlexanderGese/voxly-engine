#ifndef ENTITY_MOBS_MOB_PIG_H
#define ENTITY_MOBS_MOB_PIG_H

#include "mob_common.h"
#include "mob_rng.h"
#include "../../math/vec3.h"

// pig: passive. wanders and flees. follows the player when they hold a
// carrot (caller passes has_carrot).

void voxl_mob_pig_update(voxl_mob *m, voxl_mob_rng *r,
                         vec3 player_pos, int has_carrot, float dt);

#endif
