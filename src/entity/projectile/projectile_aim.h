#ifndef ENTITY_PROJECTILE_AIM_H
#define ENTITY_PROJECTILE_AIM_H

#include "projectile_types.h"

// aiming helpers for whoever pulls the trigger — the player's crosshair assist
// and, more usefully, skeleton AI that needs to lead a moving player and lob
// arrows over gravity. these are pure math on top of the ballistic solver, no
// world state, so the AI can call them speculatively without side effects.

// straight-line aim: just the normalized vector from->to. the dumb baseline.
vec3 projectile_aim_direct(vec3 from, vec3 to);

// lead a moving target: where to aim so a shot at muzzle speed `speed` meets a
// target currently at `tpos` moving at `tvel`. iterates time-of-flight a few
// times (it converges fast). returns the aim *point*; feed it to aim_direct or
// the arc solver. falls back to tpos if it cant converge.
vec3 projectile_aim_lead(vec3 from, vec3 tpos, vec3 tvel, float speed);

// full ballistic aim: produce a launch velocity vector that arcs a projectile of
// `kind` from `from` to a (already lead-compensated) `target` under gravity.
// writes the velocity and returns 1, or 0 if the target is out of range. picks
// the low arc. this is what an archer mob actually fires with.
int projectile_aim_ballistic(projectile_kind kind, vec3 from, vec3 target,
                             float gravity, vec3 *out_vel);

#endif
