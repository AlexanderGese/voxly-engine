#ifndef ENTITY_EPHYSICS_MOTION_H
#define ENTITY_EPHYSICS_MOTION_H

#include "ephysics_types.h"

// kinematics helpers that dont touch the world: ballistic prediction and render
// interpolation. the ai uses the prediction bits to lead a moving target with a
// thrown rock; the renderer uses the interpolation to smooth fixed-step body
// positions into a variable frame rate. both are pure math so they sit apart
// from the collision pipeline.

// position of a body after `t` seconds of pure ballistic flight (no collision),
// given a launch velocity and gravity magnitude. the obvious p + v t + 1/2 g t^2.
vec3 ephysics_motion_ballistic(vec3 pos, vec3 vel, float gravity, float t);

// time of flight for a projectile launched at `speed` from `from` to pass
// through `to`, choosing the low (flatter) arc. returns -1 if the target is out
// of range for that speed. fills `out_vel` with the launch velocity on success.
float ephysics_motion_solve_arc(vec3 from, vec3 to, float speed, float gravity,
                                vec3 *out_vel);

// lead a moving target: where to aim so a projectile of `speed` intersects a
// target moving at constant `tvel`. iterative (a couple of fixed-point passes,
// closed form is fiddly with gravity). returns the aim point.
vec3 ephysics_motion_lead(vec3 shooter, vec3 target, vec3 tvel, float speed);

// render interpolation: blend a body's previous and current fixed-step position
// by `alpha` (0..1, the fraction of the way through the current tick). plain
// lerp, but clamped so a hitch cant extrapolate past the real position.
vec3 ephysics_motion_interp(vec3 prev, vec3 cur, float alpha);

#endif
