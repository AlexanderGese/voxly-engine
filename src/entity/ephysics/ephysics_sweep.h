#ifndef ENTITY_EPHYSICS_SWEEP_H
#define ENTITY_EPHYSICS_SWEEP_H

#include "ephysics_types.h"
#include "ephysics_broadphase.h"

// swept-aabb collision. the meat. given a body, a candidate set, and a move
// vector, find the earliest time of impact and the contact normal.
//
// implementation is the standard slab method on the minkowski-expanded boxes:
// expand each block box by the body half-extents, then sweep the body *center*
// as a ray against the expanded boxes. first hit wins.

// sweep one box. returns t in [0,1] and the normal. if no hit, t=1, hit=0.
ephys_hit ephysics_sweep_box(vec3 center, vec3 half, vec3 delta, aabb block);

// sweep the whole candidate set, returns the earliest hit. caller passes the
// body's box center (pos + center_y on y) and the move delta.
ephys_hit ephysics_sweep(const ephys_candidates *c, vec3 center, vec3 half,
                         vec3 delta);

// resolve a full move against the world with up to `iters` slide passes. moves
// the body, zeroes blocked velocity components, sets WALL/CEILING/GROUNDED
// flags. does NOT do step-up (that's ephysics_step). returns remaining delta
// that couldnt be consumed (usually ~0).
vec3 ephysics_resolve(const ephys_candidates *c, ephys_body *b, vec3 delta,
                      int iters);

#endif
