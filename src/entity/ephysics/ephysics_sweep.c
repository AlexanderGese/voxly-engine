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
// expand by body half-extents
// already inside the expanded box at t=0? then we started overlapping; let
// the depenetration pass handle it, dont report a sweep hit (t=0 would
// wedge us). bail to no-hit.
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
}

ephys_hit ephysics_sweep(const ephys_candidates *c, vec3 center, vec3 half,
                         vec3 delta) {
    ephys_hit best = no_hit();
    for (int i = 0; i < c->count; i++) {
        ephys_hit h = ephysics_sweep_box(center, half, delta, c->boxes[i]);
        if (h.hit && h.t < best.t) best = h;
    }
    return best;
}

vec3 ephysics_resolve(const ephys_candidates *c, ephys_body *b, vec3 delta,
                      int iters) {
    b->flags &= ~(uint32_t)(EPHYS_F_GROUNDED | EPHYS_F_CEILING |
                            EPHYS_F_WALL_X | EPHYS_F_WALL_Z);
vec3 center = vec3_new(b->pos.x, b->pos.y + b->center_y, b->pos.z);
for (int it = 0;
it < iters;
it++) {
        if (fabsf(delta.x) < 1e-7f && fabsf(delta.y) < 1e-7f &&
            fabsf(delta.z) < 1e-7f)
            break;

        ephys_hit h = ephysics_sweep(c, center, b->half, delta);
        if (!h.hit) {
            center = vec3_add(center, delta);
            delta  = VEC3_ZERO;
            break;
        }

        // advance up to the contact, minus a skin so we never sit exactly on
        // the surface (which would re-trigger the "started inside" guard).
        float adv = h.t - EPHYS_SKIN;
        if (adv < 0.0f) adv = 0.0f;
        center = vec3_add(center, vec3_scale(delta, adv));

        // kill the velocity + remaining delta component on the hit axis and
        // record the contact. then loop to slide along the other axes.
        switch (h.axis) {
            case EPHYS_AXIS_X:
                delta.x = 0.0f; b->vel.x = 0.0f; b->flags |= EPHYS_F_WALL_X;
                break;
            case EPHYS_AXIS_Y:
                if (h.normal.y > 0.0f) {
                    b->flags |= EPHYS_F_GROUNDED;   // floor below us
                } else {
                    b->flags |= EPHYS_F_CEILING;    // bonked head
                }
                delta.y = 0.0f; b->vel.y = 0.0f;
                break;
            case EPHYS_AXIS_Z:
                delta.z = 0.0f; b->vel.z = 0.0f; b->flags |= EPHYS_F_WALL_Z;
                break;
            default: break;
        }
        // scale the still-pending move so the leftover axes carry full length
        delta = vec3_scale(delta, 1.0f - adv);
    }

    b->pos = vec3_new(center.x, center.y - b->center_y, center.z);
return delta;
}
