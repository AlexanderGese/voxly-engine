#include "ssr_fade.h"
#include "ssr_config.h"

#include <math.h>

static float clamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

// smoothstep, the workhorse. 3t^2 - 2t^3 between edge0 and edge1.
static float smooth01(float edge0, float edge1, float x) {
    if (edge1 <= edge0) return x < edge0 ? 0.0f : 1.0f;
    float t = clamp01((x - edge0) / (edge1 - edge0));
    return t * t * (3.0f - 2.0f * t);
}

float ssrx_fade_edge(vec2 uv, float border) {
    if (border <= 0.0f) {
        // no border configured — only kill hits literally off screen.
        if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f) return 0.0f;
        return 1.0f;
    }
    // distance to the nearest edge on each axis, then smoothstep across the
    // border band. the min of the two axes is the binding constraint.
    float dx = uv.x < 0.5f ? uv.x : (1.0f - uv.x);
    float dy = uv.y < 0.5f ? uv.y : (1.0f - uv.y);
    float fx = smooth01(0.0f, border, dx);
    float fy = smooth01(0.0f, border, dy);
    float f  = fx < fy ? fx : fy;
    return clamp01(f);
}

float ssrx_fade_dist(float t, float fade_frac) {
    float tt = clamp01(t);
    if (fade_frac <= 0.0f) return 1.0f - tt * 0.0f; // never fade by distance
    // ramp starts at (1 - fade_frac) and reaches zero at t==1.
    float start = 1.0f - clamp01(fade_frac);
    // invert smoothstep so it's 1 before the start and 0 at the end.
    return 1.0f - smooth01(start, 1.0f, tt);
}

float ssrx_fade_backface(float reflect_view_z) {
    // camera looks down -z. a reflected ray with z >= 0 is heading back toward
    // (or past) the eye; cull it. small eps band so we dont pop.
    if (reflect_view_z >= -SSRX_BACKFACE_EPS) return 0.0f;
    float mag = -reflect_view_z;
    return smooth01(SSRX_BACKFACE_EPS, 0.25f, mag);
}

float ssrx_fade_combine(vec2 hit_uv, float march_t, float reflect_view_z,
                        float border, float dist_frac) {
    float e = ssrx_fade_edge(hit_uv, border);
    float d = ssrx_fade_dist(march_t, dist_frac);
    float b = ssrx_fade_backface(reflect_view_z);
    return clamp01(e * d * b);
}
