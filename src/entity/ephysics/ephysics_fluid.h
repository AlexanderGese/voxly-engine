#ifndef ENTITY_EPHYSICS_FLUID_H
#define ENTITY_EPHYSICS_FLUID_H

#include "ephysics_types.h"
#include "../../world/world.h"

// fluid queries + buoyancy. water is the only fluid in the engine right now but
// i kept this generic so lava (if it ever lands) can reuse the depth math with
// a different density.

typedef struct {
    float density;    // relative to the body's reference, 1.0 == neutral
    float push;       // horizontal current strength, m/s
    vec3  flow;       // unit flow direction in the xz plane
    float surface_y;  // top of the topmost fluid cell we overlap
} ephys_fluid_state;

// predicate: does this block id behave as a fluid for physics purposes.
int ephysics_is_fluid(block_id id);

// climbable block (ladder/vines). treated like a fluid-ish medium for vertical
// control so we put it next to the fluid code rather than in the sweep.
int ephysics_is_climbable(block_id id);

// sample the fluid the body sits in. computes submerged fraction, surface
// height, and the flow vector from neighbouring fluid levels. zeroed if dry.
ephys_fluid_state ephysics_fluid_sample(world *w, const ephys_body *b);

// apply buoyancy + drag for one tick. mutates vel and sets the IN_WATER /
// SUBMERGED / fluid_h fields on the body. dt in seconds. no-op when dry.
void ephysics_fluid_apply(world *w, ephys_body *b, float dt);

#endif
