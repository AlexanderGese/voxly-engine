#ifndef ENTITY_EPHYSICS_AABB_H
#define ENTITY_EPHYSICS_AABB_H

#include "ephysics_types.h"

// small aabb helpers the sweep code leans on. the engine already has aabb.h but
// it doesnt have the minkowski-sum / overlap-depth bits i need here, and i dont
// want to bloat the math module with physics-only stuff. so it lives here.

// the world-space box for a body at its current pos.
aabb ephysics_body_box(const ephys_body *b);

// same, but at an arbitrary feet position (used for trial moves).
aabb ephysics_box_at(const ephys_body *b, vec3 feet);

// expand `a` by the half-extents of `b` -> the box `b`'s center can travel in
// without overlapping `a`. classic minkowski trick to turn the box-vs-box sweep
// into a point-vs-box sweep.
aabb ephysics_minkowski(aabb a, vec3 half);

// signed overlap depth of two boxes per axis. negative means separated on that
// axis. used to push apart bodies that start the tick already intersecting.
vec3 ephysics_overlap_depth(aabb a, aabb b);

// 1 if the two boxes overlap with a tiny epsilon slop, else 0.
int  ephysics_touching(aabb a, aabb b, float eps);

#endif
