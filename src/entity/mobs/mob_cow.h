#ifndef ENTITY_MOBS_MOB_COW_H
#define ENTITY_MOBS_MOB_COW_H

#include "mob_common.h"
#include "mob_rng.h"
#include "../../math/vec3.h"

// cow: passive grazer. wanders, flees when hit. can be milked (caller checks
// the cooldown via voxl_mob_cow_can_milk).

void voxl_mob_cow_update(voxl_mob *m, voxl_mob_rng *r, vec3 player_pos, float dt);

// cows can be milked again after a cooldown; returns 1 if ready.
int  voxl_mob_cow_can_milk(const voxl_mob *m);

// mark this cow as just milked (starts the cooldown).
void voxl_mob_cow_milked(voxl_mob *m);

#endif
