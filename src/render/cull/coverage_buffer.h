#ifndef RENDER_CULL_COVERAGE_H
#define RENDER_CULL_COVERAGE_H

#include "../../math/mat4.h"
#include "../../math/aabb.h"
#include "../../math/vec3.h"

// a tiny software depth buffer used for occlusion culling. we rasterize
// the front (occluder) chunks into a low-res depth grid, then test the
// boxes of everything behind against it. cpu side, single threaded,
// deliberately small — this is a coarse reject, not a real z-buffer.
//
// depth stored as the nearest (smallest) clip-w-ish value seen per cell.
// we keep MAX depth = far, and writing a closer occluder lowers it.

#define CCOV_W   160
#define CCOV_H    90
#define CCOV_FAR  1.0e30f

typedef struct {
    int   w, h;
    float depth[CCOV_W * CCOV_H];   // nearest depth per cell
    mat4  vp;                       // view-proj used to project boxes
    int   any_occluders;            // did we actually write anything
} cull_coverage;

// reset to empty (all far). call once per frame before drawing occluders.
void cull_coverage_clear(cull_coverage *cb, mat4 vp);

// rasterize an aabb as a solid screen-space rectangle at its nearest
// depth. used to seed occluders from already-known-visible solid chunks.
// returns 1 if anything was written.
int  cull_coverage_add_box(cull_coverage *cb, aabb a);

// test an aabb: returns 1 if it is fully occluded (every covered cell is
// already closer than the box's nearest depth), 0 if any part might show.
// conservative — when in doubt it says visible.
int  cull_coverage_test_box(const cull_coverage *cb, aabb a);

#endif
