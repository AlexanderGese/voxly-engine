#ifndef RENDER_CULL_OCCLUSION_H
#define RENDER_CULL_OCCLUSION_H

#include "coverage_buffer.h"
#include "cull_types.h"

// occlusion culling pass. the idea: chunks are sorted front-to-back, we
// walk them near->far, drawing each survivor into the coverage buffer as
// an occluder and testing the next ones against what's already there.
// classic "render front, reject behind" software occlusion.
//
// this is approximate. it will never wrongly cull something that IS
// visible (conservative), but it can miss some occlusions because the
// coverage buffer is coarse and chunks are boxes not actual meshes.

typedef struct {
    cull_coverage cov;
    int   enabled;
    int   min_occluder_solid;   // skip thin/empty chunks as occluders
    int   tested;               // counters for the frame
    int   rejected;
} cull_occluder;

void cull_occlusion_begin(cull_occluder *oc, mat4 vp, int enabled);

// process one item that already passed frustum + distance and is sorted
// front-to-back. mutates item->reason. returns 1 if it survives (should be
// drawn) and was registered as an occluder, 0 if it got culled.
int  cull_occlusion_step(cull_occluder *oc, cull_item *item);

// convenience: run the whole pass over an already-sorted item array.
// returns number of survivors. items keep their reason field updated.
int  cull_occlusion_run(cull_occluder *oc, cull_item *items, int count);

#endif
