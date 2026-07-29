#ifndef ENTITY_MOBS_BEHAVIOR_CHASE_H
#define ENTITY_MOBS_BEHAVIOR_CHASE_H

#include "mob_common.h"
#include "../../math/vec3.h"
#include <stdbool.h>

// pursue a target point (usually the player's position).

// turn toward `target` and walk at `speed`. returns flat distance to target.
float voxl_mob_chase_tick(voxl_mob *m, vec3 target, float speed, float dt);

// should this mob give up chasing? true if target is further than `lose`.
bool voxl_mob_chase_should_give_up(const voxl_mob *m, vec3 target, float lose);

// in attack range? true if flat distance <= `reach`.
bool voxl_mob_chase_in_reach(const voxl_mob *m, vec3 target, float reach);

#endif
