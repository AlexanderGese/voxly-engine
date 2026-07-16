#ifndef RENDER_SKYBOX_FOG_H
#define RENDER_SKYBOX_FOG_H

// distance fog tuned to blend the world into the sky dome. the trick that
// makes voxel worlds not look like they end at a wall: fog color isn't a fixed
// gray, it's the sky color in the view direction, so geometry dissolves into
// whatever the dome shows behind it.
//
// this produces fog params the block shader / cpu blend can consume, plus a
// helper to compute the per-fragment-ish blend factor on the cpu side for
// things we shade by hand (particles, water, distant billboards).

#include "skyb_common.h"
#include "skyb_gradient.h"

typedef struct {
    float    start;     // world distance where fog begins
    float    end;       // distance where fog is full
    float    density;   // exp fog density (used by exp mode)
    int      exp_mode;  // 0 linear, 1 exp2
    skyb_rgb base;      // fallback fog color (sky-sampled at view dir wins)
} skyb_fog;

// derive fog from the view distance + current sky. `view_dist` is the far
// plane in blocks (RENDER_DISTANCE*CHUNK_SIZE_X). weather raises density and
// pulls `start` inward; `wetness` 0..1 is rain/snow thickness.
skyb_fog skyb_fog_from_sky(float view_dist, const skyb_gradient *grad,
                           float wetness);

// 0..1 fog amount for a fragment at `dist` blocks away. honors exp/linear.
float skyb_fog_factor(const skyb_fog *f, float dist);

// the color fog should be when looking along `view_dir`. samples the sky dome
// gradient so distant geometry melts into the actual sky, falling back to the
// flat base if no gradient is wired.
skyb_rgb skyb_fog_color(const skyb_fog *f, const skyb_gradient *grad,
                        vec3 view_dir);

// convenience: blend a surface color toward the fog color by the distance
// factor. one call for the hand-shaded paths.
skyb_rgb skyb_fog_apply(const skyb_fog *f, const skyb_gradient *grad,
                        skyb_rgb surface, vec3 view_dir, float dist);

#endif
