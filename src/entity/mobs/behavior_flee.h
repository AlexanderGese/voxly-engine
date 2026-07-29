#ifndef ENTITY_MOBS_BEHAVIOR_FLEE_H
#define ENTITY_MOBS_BEHAVIOR_FLEE_H

#include "mob_common.h"
#include "../../math/vec3.h"
#include <stdbool.h>

// run away from a threat point. used by passive mobs after being hurt.

// turn away from `threat` and sprint at `speed`. returns flat dist to threat.
float voxl_mob_flee_tick(voxl_mob *m, vec3 threat, float speed, float dt);

// far enough to calm down? true if flat distance >= `safe`.
bool voxl_mob_flee_is_safe(const voxl_mob *m, vec3 threat, float safe);

#endif
