#ifndef ENTITY_MOBS_MOB_CHICKEN_H
#define ENTITY_MOBS_MOB_CHICKEN_H

#include "mob_common.h"
#include "mob_rng.h"
#include "../../math/vec3.h"

// chicken: passive, light. wanders/flees like other animals but flaps to
// break its fall (reduced gravity while airborne) and lays eggs on a timer.

void voxl_mob_chicken_update(voxl_mob *m, voxl_mob_rng *r,
                             vec3 player_pos, float dt);

// returns 1 once the egg timer elapses, and resets it. caller drops an egg.
int  voxl_mob_chicken_lay_egg(voxl_mob *m, voxl_mob_rng *r, float dt);

#endif
