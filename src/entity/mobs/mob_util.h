#ifndef ENTITY_MOBS_MOB_UTIL_H
#define ENTITY_MOBS_MOB_UTIL_H

#include "mob_common.h"
#include "../../math/vec3.h"
#include <stdbool.h>

// small shared helpers for the mob modules. pure data, no engine deps.

// flat (xz) distance between two points. ignores y so a mob standing on a
// hill still "reaches" a player below it.
float voxl_mob_flat_dist(vec3 a, vec3 b);

// squared flat distance, for cheap range checks.
float voxl_mob_flat_dist_sq(vec3 a, vec3 b);

// yaw (radians) that points from `from` toward `to` on the xz plane.
// matches the engine convention: yaw 0 faces -z.
float voxl_mob_yaw_to(vec3 from, vec3 to);

// turn `cur` yaw toward `want` yaw by at most max_step radians.
// handles wraparound so we never spin the long way round.
float voxl_mob_turn_toward(float cur, float want, float max_step);

// unit xz heading for a yaw. y is always 0.
vec3 voxl_mob_heading(float yaw);

// move the mob forward along its current yaw at `speed` m/s for dt seconds.
void voxl_mob_walk_forward(voxl_mob *m, float speed, float dt);

// clamp helper (floats).
float voxl_mob_clampf(float v, float lo, float hi);

// wrap an angle into [-pi, pi].
float voxl_mob_wrap_angle(float a);

// is this kind hostile (chases + attacks the player)?
bool voxl_mob_is_hostile(voxl_mob_kind k);

// is this kind passive prey (flees when hurt)?
bool voxl_mob_is_passive(voxl_mob_kind k);

#endif
