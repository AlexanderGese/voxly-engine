#ifndef ENTITY_PROJECTILE_BALLISTIC_H
#define ENTITY_PROJECTILE_BALLISTIC_H

#include "projectile_types.h"

// the motion half of a projectile tick: gravity, air drag, terminal velocity,
// heading tracking. no collision in here — this just proposes where the thing
// *wants* to be next; projectile_collide gets to veto it. splitting them keeps
// each testable: you can integrate a thousand steps with collision stubbed out
// and check the arc matches the closed-form parabola within epsilon.

// advance one projectile by dt under its kind's forces. mutates vel + forward
// in place and writes the *unclamped* proposed position to out_next. does NOT
// touch p->pos — the collide pass owns committing position so a partial step
// can stop short at a wall.
void projectile_ballistic_step(projectile *p, float dt, vec3 *out_next);

// the analytic arc helper the aim assist + fuzzer share: where does a shot from
// origin with this initial velocity land on the y=ground_y plane, ignoring drag
// and collision? returns 0 if it never comes down (going up forever, no
// gravity). handy for "lead the target" math without simulating.
int projectile_ballistic_predict(vec3 origin, vec3 vel, float gravity,
                                 float ground_y, vec3 *out_hit, float *out_t);

// solve the launch *pitch* (radians) needed to hit a target at horizontal
// distance d and height dy with muzzle speed v under gravity g. low arc. writes
// the angle and returns 1, or returns 0 if the target is out of range.
int projectile_ballistic_solve_pitch(float d, float dy, float v, float g,
                                     float *out_pitch);

#endif
