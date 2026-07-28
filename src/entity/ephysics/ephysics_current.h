#ifndef ENTITY_EPHYSICS_CURRENT_H
#define ENTITY_EPHYSICS_CURRENT_H

#include "ephysics_types.h"
#include "../../world/world.h"

// water currents. the fluid pass in ephysics_fluid handles the *vertical*
// buoyancy/drag; this is the horizontal push you get from flowing water. flowing
// fluid cells have a level gradient (full source next to a half-full neighbour),
// and the flow runs downhill along that gradient. minecraft does the same thing,
// it's what lets water pin you against a wall or carry you down a stream.
//
// we sample the gradient from the world's block levels and turn it into a small
// per-tick velocity nudge, weighted by how much of the body is submerged.

// the resolved flow for a body: a unit-ish xz direction and a strength in m/s.
typedef struct {
    vec3  dir;        // flow direction, xz plane, roughly unit (0 if still)
    float strength;   // push magnitude this tick, m/s before submersion weight
    float submersion; // 0..1 fraction of the body in fluid, scales the push
} ephys_current;

// sample the current acting on the body from the surrounding fluid cells.
// returns a zeroed current when the body isnt in flowing water.
ephys_current ephysics_current_sample(world *w, const ephys_body *b);

// apply a sampled current to the body's horizontal velocity for one tick.
// mutates b->vel.{x,z}. the push is additive and capped so a fast river cant
// accelerate you past the sweep's safe speed.
void ephysics_current_apply(ephys_body *b, const ephys_current *cur, float dt);

#endif
