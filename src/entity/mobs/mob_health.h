#ifndef ENTITY_MOBS_MOB_HEALTH_H
#define ENTITY_MOBS_MOB_HEALTH_H

#include "mob_common.h"
#include "../../math/vec3.h"
#include <stdbool.h>

// damage, knockback and death bookkeeping shared by every mob.

// apply `amount` damage from a hit coming from `source` position.
// sets the hurt timer, applies knockback to velocity, and flips the mob
// into VOXL_MS_DEAD if health hits 0. returns true if the hit was lethal.
bool voxl_mob_hurt(voxl_mob *m, int amount, vec3 source, float knockback);

// heal up to max_health. negative amounts are ignored.
void voxl_mob_heal(voxl_mob *m, int amount);

// is the mob dead (health gone or already in the dead state)?
bool voxl_mob_is_dead(const voxl_mob *m);

// tick down the hurt flash timer. call once per frame.
void voxl_mob_health_tick(voxl_mob *m, float dt);

#endif
