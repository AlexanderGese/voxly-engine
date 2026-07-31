#ifndef ENTITY_PROJECTILE_AIM_H
#define ENTITY_PROJECTILE_AIM_H
#include "projectile_types.h"
// aiming helpers for whoever pulls the trigger — the player's crosshair assist
// and, more usefully, skeleton AI that needs to lead a moving player and lob
// arrows over gravity. these are pure math on top of the ballistic solver, no
// world state, so the AI can call them speculatively without side effects.
// straight-line aim: just the normalized vector from->to. the dumb baseline.
vec3 projectile_aim_direct(vec3 from, vec3 to);
vec3 projectile_aim_lead(vec3 from, vec3 tpos, vec3 tvel, float speed);
int projectile_aim_ballistic(projectile_kind kind, vec3 from, vec3 target,
                             float gravity, vec3 *out_vel);
#endif
