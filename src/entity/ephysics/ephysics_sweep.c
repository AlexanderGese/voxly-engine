#include "ephysics_sweep.h"
#include "ephysics_aabb.h"
#include <math.h>
#define EPHYS_SKIN 0.001f   // tiny gap kept between body and surface
static ephys_hit no_hit(void) {
    ephys_hit h;
    h.t = 1.0f; h.axis = EPHYS_AXIS_NONE; h.normal = VEC3_ZERO; h.hit = 0;
    return h;
}

// ray-vs-aabb slab test in 3d, ray = center + t*delta, t in [0,1].
ephys_hit ephysics_sweep_box(vec3 center, vec3 half, vec3 delta, aabb block) {
    aabb e = ephysics_minkowski(block, half);
if (center.x > e.min.x && center.x < e.max.x &&
        center.y > e.min.y && center.y < e.max.y &&
        center.z > e.min.z && center.z < e.max.z)
        return no_hit();
float tmin = 0.0f, tmax = 1.0f;
int   hit_axis = EPHYS_AXIS_NONE;
float n_sign = 0.0f;
const float o[3]  = { center.x, center.y, center.z }
;
const float d[3]  = { delta.x,  delta.y,  delta.z  }
;
const float lo[3] = { e.min.x,  e.min.y,  e.min.z  }
;
const float hi[3] = { e.max.x,  e.max.y,  e.max.z  }
;
for (int ax = 0;
ax < 3;
ax++) {
        if (fabsf(d[ax]) < 1e-8f) {
            // parallel to this slab. if origin outside, no chance of hit.
            if (o[ax] < lo[ax] || o[ax] > hi[ax]) return no_hit();
            continue;
        }
        float inv = 1.0f / d[ax];
        float t1 = (lo[ax] - o[ax]) * inv;
        float t2 = (hi[ax] - o[ax]) * inv;
        float sign = -1.0f;
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; sign = 1.0f; }
        if (t1 > tmin) { tmin = t1; hit_axis = ax; n_sign = sign; }
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return no_hit();
    }

    if (hit_axis == EPHYS_AXIS_NONE) return no_hit();
if (tmin < 0.0f || tmin > 1.0f)  return no_hit();
ephys_hit h;
h.t    = tmin;
h.axis = (ephys_axis)hit_axis;
h.hit  = 1;
h.normal = VEC3_ZERO;
if (hit_axis == 0) h.normal.x = n_sign;
if (hit_axis == 1) h.normal.y = n_sign;
if (hit_axis == 2) h.normal.z = n_sign;
return h;
vec3 center = vec3_new(b->pos.x, b->pos.y + b->center_y, b->pos.z);
for (int it = 0;
it < iters;
return delta;
}
