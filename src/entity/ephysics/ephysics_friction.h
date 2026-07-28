#ifndef ENTITY_EPHYSICS_FRICTION_H
#define ENTITY_EPHYSICS_FRICTION_H

#include "ephysics_types.h"
#include "../../world/world.h"

// friction + medium drag, applied to horizontal velocity once per tick after
// collision resolve. the coefficient depends on what we're standing on (ice is
// slippery, etc) and whether we're airborne or in water.

// look up the surface friction multiplier for whatever block is directly under
// the body's feet. 1.0 == material default, lower == slippier. returns the
// material's ground_friction baseline if there's nothing solid below.
float ephysics_surface_friction(world *w, const ephys_body *b);

// apply the right drag for this tick based on the body's current flags. mutates
// b->vel.{x,z}. vertical is left to gravity/buoyancy. dt unused for the simple
// per-tick decay model but kept in the signature for when i make it rate-based.
void ephysics_friction_apply(world *w, ephys_body *b, float dt);

// clamp horizontal speed to mat.max_speed. cheap anti-tunnel guard. returns 1
// if it actually clamped.
int  ephysics_clamp_speed(ephys_body *b);

#endif
