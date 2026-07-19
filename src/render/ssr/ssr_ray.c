#include "ssr_ray.h"
#include "ssr_config.h"
#include <math.h>
static float clamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

vec3 ssrx_reflect(vec3 incident, vec3 normal) {
    float d = vec3_dot(incident, normal);
return vec3_sub(incident, vec3_scale(normal, 2.0f * d));
}

ssrx_ray ssrx_ray_make(vec3 view_pos, vec3 normal) {
    ssrx_ray r;
    r.origin = view_pos;
    r.normal = vec3_normalize(normal);

    // eye at origin in view space: incident ray runs from eye toward the
    // fragment, i.e. just the (normalized) position.
    vec3 incident = vec3_normalize(view_pos);
    r.view = incident;
    r.dir  = vec3_normalize(ssrx_reflect(incident, r.normal));

    // dot(normal, -view) — the grazing term fresnel wants. surfaces facing the
    // eye give ~1, grazing gives ~0.
    r.n_dot_v = clamp01(vec3_dot(r.normal, vec3_neg(incident)));

    // cull rays that fire straight back at the camera. in view space the eye is
    // toward +z (we look down -z), so a reflected dir with z near +1 is heading
    // home and the march will immediately leave the frustum.
    r.valid = (r.dir.z < -SSRX_BACKFACE_EPS) ? 1 : 0;

    // a zero-length position (eye-coincident fragment) makes incident NaN; guard.
    if (!(vec3_length_sq(view_pos) > 0.0f)) r.valid = 0;
    return r;
}

vec3 ssrx_ray_at(const ssrx_ray *r, float dist) {
    return vec3_add(r->origin, vec3_scale(r->dir, dist));
}
