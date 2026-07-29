#ifndef ENTITY_MOBS_BEHAVIOR_WANDER_H
#define ENTITY_MOBS_BEHAVIOR_WANDER_H

#include "mob_common.h"
#include "mob_rng.h"

// idle/wander behavior. picks a random nearby point, ambles toward it,
// then picks a new one. returns nothing; mutates the mob in place.

// pick a fresh wander target within `radius` of the mob's current pos.
void voxl_mob_wander_pick(voxl_mob *m, voxl_mob_rng *r, float radius);

// advance the wander behavior by dt at the given walk speed.
// if there is no target (or we arrived / timed out) a new one is chosen.
void voxl_mob_wander_tick(voxl_mob *m, voxl_mob_rng *r,
                          float speed, float radius, float dt);

#endif
