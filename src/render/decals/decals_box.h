#ifndef RENDER_DECALS_BOX_H
#define RENDER_DECALS_BOX_H
#include "decals_types.h"
// projector box construction + the math that turns a world point into the
// box's local cube space. kept separate from the pool because it's pure and
// i wanted to be able to unit-test it without dragging gl in.
// build a projector from a center, a forward (projection) axis and an up hint.
// `size` is full width/height/depth, not half. returns the assembled projector
decals_projector decals_box_make(vec3 center, vec3 forward, vec3 up_hint, vec3 size);
void decals_box_refresh(decals_projector *p);
vec3 decals_box_to_local(const decals_projector *p, vec3 world);
int decals_box_contains(const decals_projector *p, vec3 world);
void decals_box_local_uv(const decals_atlas_region *r, float lx, float ly,
                         float *u, float *v);
#endif
