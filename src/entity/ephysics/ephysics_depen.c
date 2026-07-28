#include "ephysics_depen.h"
#include "ephysics_aabb.h"
#include "ephysics_query.h"

#include <math.h>

#define EPHYS_DEPEN_EPS 0.0005f

int ephysics_depen_overlapping(const ephys_candidates *c, const ephys_body *b) {
    aabb box = ephysics_body_box(b);
    for (int i = 0; i < c->count; i++)
        if (aabb_intersects(box, c->boxes[i])) return 1;
    return 0;
}

// pick the single axis/direction that escapes box `bb` with the least travel.
// returns the signed push distance and which axis via *axis.
static float min_translation(aabb body, aabb bb, ephys_axis *axis) {
    vec3 d = ephysics_overlap_depth(bb, body);   // how to push body out of bb

    float ax = fabsf(d.x), ay = fabsf(d.y), az = fabsf(d.z);

    // any non-overlap on an axis means they're actually separated -> no push.
    if (ax <= 0.0f || ay <= 0.0f || az <= 0.0f) {
        *axis = EPHYS_AXIS_NONE;
        return 0.0f;
    }

    if (ax <= ay && ax <= az) { *axis = EPHYS_AXIS_X; return d.x; }
    if (ay <= az)             { *axis = EPHYS_AXIS_Y; return d.y; }
    *axis = EPHYS_AXIS_Z;
    return d.z;
}

float ephysics_depen_resolve(const ephys_candidates *c, ephys_body *b,
                             int max_iters) {
    float total = 0.0f;

    for (int it = 0; it < max_iters; it++) {
        aabb box = ephysics_body_box(b);

        // find the deepest overlapping box this pass and push out of that one.
        // doing the deepest first converges faster than going in list order.
        int   worst = -1;
        float worst_depth = 0.0f;
        ephys_axis worst_axis = EPHYS_AXIS_NONE;
        float worst_push = 0.0f;

        for (int i = 0; i < c->count; i++) {
            if (!aabb_intersects(box, c->boxes[i])) continue;
            ephys_axis ax;
            float push = min_translation(box, c->boxes[i], &ax);
            if (ax == EPHYS_AXIS_NONE) continue;
            if (fabsf(push) > worst_depth) {
                worst_depth = fabsf(push);
                worst       = i;
                worst_axis  = ax;
                worst_push  = push;
            }
        }

        if (worst < 0) break;   // clean

        // nudge a hair past the surface so the next overlap test reads clear.
        float push = worst_push + (worst_push > 0 ? EPHYS_DEPEN_EPS
                                                  : -EPHYS_DEPEN_EPS);
        switch (worst_axis) {
            case EPHYS_AXIS_X: b->pos.x += push; b->vel.x = 0.0f; break;
            case EPHYS_AXIS_Y: b->pos.y += push; b->vel.y = 0.0f; break;
            case EPHYS_AXIS_Z: b->pos.z += push; b->vel.z = 0.0f; break;
            default: break;
        }
        total += fabsf(push);
    }

    return total;
}

float ephysics_depen_eject_up(world *w, ephys_body *b, float max_up) {
    // last-ditch unstick. used when depen_resolve cant make progress because the
    // body is boxed in on the cheap axes. we just rise until the body box is
    // clear of solid blocks or we hit the budget.
    float moved = 0.0f;
    const float step = 0.25f;

    while (moved < max_up) {
        aabb box = ephysics_body_box(b);
        if (!ephysics_query_box_solid(w, box)) break;
        b->pos.y += step;
        moved    += step;
    }
    if (moved > 0.0f) b->vel.y = 0.0f;
    return moved;
}
