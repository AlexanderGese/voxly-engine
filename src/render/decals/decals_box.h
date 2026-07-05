#ifndef RENDER_DECALS_BOX_H
#define RENDER_DECALS_BOX_H

#include "decals_types.h"

// projector box construction + the math that turns a world point into the
// box's local cube space. kept separate from the pool because it's pure and
// i wanted to be able to unit-test it without dragging gl in.

// build a projector from a center, a forward (projection) axis and an up hint.
// `size` is full width/height/depth, not half. returns the assembled projector
// with the basis orthonormalised and the cached matrix + bounds filled in.
decals_projector decals_box_make(vec3 center, vec3 forward, vec3 up_hint, vec3 size);

// recompute world_to_local and bounds. call after poking center/basis/half by
// hand. decals_box_make already does this for you.
void decals_box_refresh(decals_projector *p);

// map a world point into local space. inside the box <=> all components within
// [-0.5, 0.5]. handy for the cpu-side "does this decal touch this point" tests.
vec3 decals_box_to_local(const decals_projector *p, vec3 world);

// 1 if the world point lands inside the projector cube.
int decals_box_contains(const decals_projector *p, vec3 world);

// local-cube xy -> atlas uv inside [uv0,uv1]. local xy is expected in
// [-0.5,0.5]; out of range is clamped (the projector edges shouldnt show
// stretched garbage).
void decals_box_local_uv(const decals_atlas_region *r, float lx, float ly,
                         float *u, float *v);

#endif
