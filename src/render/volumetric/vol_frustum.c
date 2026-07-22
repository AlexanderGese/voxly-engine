#include "vol_frustum.h"

#include <math.h>

// slab method, done by hand so we get *both* the entry and exit t (the engine's
// aabb_ray only returns the near hit). standard ray/aabb: for each axis compute
// the t range where the ray is inside the slab, intersect the three ranges.

volumetric_segment volumetric_frustum_clip(aabb box, vec3 origin, vec3 dir,
                                           float max_t) {
    volumetric_segment seg = { 0.0f, 0.0f, 0 };

    float tmin = 0.0f;          // never march behind the eye
    float tmax = max_t;         // never march past the surface

    const float o[3] = { origin.x, origin.y, origin.z };
    const float d[3] = { dir.x, dir.y, dir.z };
    const float lo[3] = { box.min.x, box.min.y, box.min.z };
    const float hi[3] = { box.max.x, box.max.y, box.max.z };

    for (int a = 0; a < 3; a++) {
        if (fabsf(d[a]) < 1e-8f) {
            // ray parallel to this slab — if the origin is outside it, no hit.
            if (o[a] < lo[a] || o[a] > hi[a]) return seg;
            continue;
        }
        float inv = 1.0f / d[a];
        float t1 = (lo[a] - o[a]) * inv;
        float t2 = (hi[a] - o[a]) * inv;
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }   // order them
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return seg;   // intervals diverged, miss
    }

    seg.t_near = tmin;
    seg.t_far  = tmax;
    seg.hit    = (tmax > tmin) ? 1 : 0;
    return seg;
}

aabb volumetric_frustum_bounds(vec3 camera, float half_extent) {
    if (half_extent < 0.0f) half_extent = 0.0f;
    vec3 half = vec3_new(half_extent, half_extent, half_extent);
    return aabb_from_center(camera, half);
}
