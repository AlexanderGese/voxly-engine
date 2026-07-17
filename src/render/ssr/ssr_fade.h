#ifndef RENDER_SSR_FADE_H
#define RENDER_SSR_FADE_H

#include "../../math/vec2.h"

// the two confidence terms that keep ssr from showing garbage:
//
// edge fade  — a hit near the screen border has almost no neighbouring data,
// and rays that walk off the side return nothing at all. fade the
// contribution out as the hit uv nears any edge.
//
// dist fade  — long marches drift further from being physically correct (the
// depth buffer is a height field, not real geometry) so taper the
// far end of the ray.
//
// both return a multiplier in [0,1]. multiply them together for the final
// confidence weight.

// edge fade from a hit uv. `border` is the fade width per side as a fraction of
// the screen (0..0.5). 1 in the middle, 0 at/over the border.
float ssrx_fade_edge(vec2 uv, float border);

// distance fade. `t` is the normalized march progress 0..1 (0 = at the origin,
// 1 = end of the longest allowed march). `fade_frac` is the fraction of the
// march length at the far end over which we ramp to zero.
float ssrx_fade_dist(float t, float fade_frac);

// the ray also points back toward the camera sometimes (reflection nearly
// parallel to the screen). those marches are unstable; this gives a soft cull
// based on the reflected ray's z (view space, camera looks down -z). returns 0
// for rays heading straight at the eye, ramps to 1 as they head away.
float ssrx_fade_backface(float reflect_view_z);

// convenience: combine all three. saves the call site juggling three numbers.
float ssrx_fade_combine(vec2 hit_uv, float march_t, float reflect_view_z,
                        float border, float dist_frac);

#endif
