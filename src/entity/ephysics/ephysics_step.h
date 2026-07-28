#ifndef ENTITY_EPHYSICS_STEP_H
#define ENTITY_EPHYSICS_STEP_H

#include "ephysics_types.h"
#include "ephysics_broadphase.h"

// step-up. when a horizontal move gets blocked by a wall but there's a ledge
// no taller than mat.step_height, we try to climb it instead of stopping dead.
// the trick: do the horizontal move at a raised position, and if that succeeds
// further than the flat move did, drop back down onto the ledge.

// attempt to step `b` up over a ledge for the horizontal part of `delta`.
// `flat_pos` is where the normal flat resolve landed. returns 1 if a step was
// taken (b->pos updated, EPHYS_F_STEPPED set), 0 if no better path was found.
int ephysics_step(const ephys_candidates *c, ephys_body *b, vec3 delta,
                  vec3 flat_pos);

// drop the body straight down until it rests on something, but no further than
// `max_drop`. used after a step to snap onto the ledge surface. returns the
// distance dropped.
float ephysics_settle(const ephys_candidates *c, ephys_body *b, float max_drop);

#endif
