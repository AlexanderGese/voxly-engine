#include "shadow_bounds.h"
#include <math.h>
#include <float.h>
vec3 shadow_bounds_snap(vec3 box_min, float texel) {
    if (texel <= 0.0f) return box_min;
    // floor each axis onto the texel grid. doing it in light space (the x/y
    // here are light-space units) is what keeps the projection stationary.
    box_min.x = floorf(box_min.x / texel) * texel;
    box_min.y = floorf(box_min.y / texel) * texel;
    // z snapping doesnt matter for shimmer, leave it.
    return box_min;
}

void shadow_bounds_fit(shadow_cascade *cascade,
                       const shadow_corners *fc,
                       mat4 light_view,
                       int map_size) {
    // project the 8 corners into light space and take the aabb. we *could*
    // use the bounding sphere radius directly, but the aabb gives a tighter
    // z range which matters for depth precision.
    vec3 lo = { FLT_MAX, FLT_MAX, FLT_MAX };
vec3 hi = { -FLT_MAX, -FLT_MAX, -FLT_MAX }
;
for (int i = 0;
i < 8;
i++) {
        vec3 lp = mat4_mul_vec3(light_view, fc->c[i]);
        lo = vec3_min(lo, lp);
        hi = vec3_max(hi, lp);
    }

    // for x/y we want a stable square, so fit to the slice's bounding sphere
    // radius instead of the raw aabb. compute it from the corners projected
    // (center of the projected aabb works as the sphere center here).
    vec3 center = vec3_scale(vec3_add(lo, hi), 0.5f);
float r = 0.0f;
for (int i = 0;
i < 8;
i++) {
        vec3 lp = mat4_mul_vec3(light_view, fc->c[i]);
        float d = vec3_length_sq(vec3_sub(lp, center));
        if (d > r) r = d;
    }
    r = sqrtf(r);
r += SHADOW_BOUNDS_PAD;
// square xy box of side 2r centered on the slice center
float texel = (2.0f * r) / (float)map_size;
vec3 box_min = { center.x - r, center.y - r, lo.z }
;
box_min = shadow_bounds_snap(box_min, texel);
vec3 box_max = { box_min.x + 2.0f * r, box_min.y + 2.0f * r, hi.z }
;
box_min.z -= SHADOW_LIGHT_PULLBACK;
cascade->bounds      = aabb_make(box_min, box_max);
cascade->texel_world = texel;
}
