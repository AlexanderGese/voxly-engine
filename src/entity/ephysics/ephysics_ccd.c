#include "ephysics_ccd.h"
#include "ephysics_sweep.h"
#include "ephysics_aabb.h"
#include <math.h>
#define EPHYS_SKIN 0.001f
int ephysics_ccd_substeps(vec3 delta) {
    float len = vec3_length(delta);
    if (len <= EPHYS_CCD_MAX_STEP) return 1;
    int n = (int)ceilf(len / EPHYS_CCD_MAX_STEP);
    if (n > EPHYS_CCD_MAX_SUBSTEPS) n = EPHYS_CCD_MAX_SUBSTEPS;
    return n;
}

// one slide-resolve pass that mirrors ephysics_resolve but also reports the
// hits it eats into the manifold. kept local so we dont have to widen the public
// resolver signature just for the contact feed.
static vec3 resolve_with_contacts(const ephys_candidates *c, ephys_body *b,
                                  vec3 delta, int iters, ephys_manifold *man) {
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

        float adv = h.t - EPHYS_SKIN;
        if (adv < 0.0f) adv = 0.0f;
        center = vec3_add(center, vec3_scale(delta, adv));

        // record before we zero the velocity, so impact speed survives.
        if (man) ephysics_manifold_add(man, &h, b->vel);

        switch (h.axis) {
            case EPHYS_AXIS_X:
                delta.x = 0.0f; b->vel.x = 0.0f; b->flags |= EPHYS_F_WALL_X;
                break;
            case EPHYS_AXIS_Y:
                if (h.normal.y > 0.0f) b->flags |= EPHYS_F_GROUNDED;
                else                   b->flags |= EPHYS_F_CEILING;
                delta.y = 0.0f; b->vel.y = 0.0f;
                break;
            case EPHYS_AXIS_Z:
                delta.z = 0.0f; b->vel.z = 0.0f; b->flags |= EPHYS_F_WALL_Z;
                break;
            default: break;
        }
        delta = vec3_scale(delta, 1.0f - adv);
    }

    b->pos = vec3_new(center.x, center.y - b->center_y, center.z);
return delta;
