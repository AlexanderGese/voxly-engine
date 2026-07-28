#include "ephysics_fluid.h"
#include "ephysics_aabb.h"
#include "ephysics_broadphase.h"
#include "../../world/block.h"
#include "../../config.h"

#include <math.h>

int ephysics_is_fluid(block_id id) {
    return id == BLOCK_WATER;   // only fluid we have. lava someday.
}

int ephysics_is_climbable(block_id id) {
    return id == 36 /*ladder*/;   // ext id, see block_ext.h BLOCK_LADDER
}

// how much of the body box (by height) is below the fluid surface, 0..1.
static float submerged_fraction(const ephys_body *b, float surface_y) {
    aabb box = ephysics_body_box(b);
    float lo = box.min.y, hi = box.max.y;
    if (surface_y <= lo) return 0.0f;
    if (surface_y >= hi) return 1.0f;
    float span = hi - lo;
    if (span <= 1e-5f) return 0.0f;
    return (surface_y - lo) / span;
}

ephys_fluid_state ephysics_fluid_sample(world *w, const ephys_body *b) {
    ephys_fluid_state fs;
    fs.density   = 0.0f;
    fs.push      = 0.0f;
    fs.flow      = VEC3_ZERO;
    fs.surface_y = -1e9f;

    ephys_candidates c;
    ephysics_gather_fluid(w, b, &c);
    if (c.count == 0) return fs;

    // surface is the highest top face among overlapping fluid cells.
    for (int i = 0; i < c.count; i++)
        if (c.boxes[i].max.y > fs.surface_y) fs.surface_y = c.boxes[i].max.y;

    fs.density = 1.0f;

    // crude flow: look at the 4 horizontal neighbours of the feet cell. fluid
    // "wants" to move toward the neighbour that has air below it (a drop). this
    // isnt the real fluid sim's notion of level but it reads fine for pushing
    // entities downstream.
    int fx = (int)floorf(b->pos.x);
    int fy = (int)floorf(b->pos.y);
    int fz = (int)floorf(b->pos.z);
    vec3 flow = VEC3_ZERO;
    static const int dx[4] = { 1, -1, 0, 0 };
    static const int dz[4] = { 0, 0, 1, -1 };
    for (int k = 0; k < 4; k++) {
        block_id side  = world_get_block(w, fx + dx[k], fy, fz + dz[k]);
        block_id below = world_get_block(w, fx + dx[k], fy - 1, fz + dz[k]);
        if (!ephysics_is_fluid(side) && !block_is_solid(side)) {
            // open space to the side: current runs that way
            flow.x += (float)dx[k];
            flow.z += (float)dz[k];
        }
        if (ephysics_is_fluid(side) && block_is_air(below)) {
            flow.x += (float)dx[k] * 0.5f;
            flow.z += (float)dz[k] * 0.5f;
        }
    }
    if (fabsf(flow.x) > 1e-4f || fabsf(flow.z) > 1e-4f) {
        fs.flow = vec3_normalize(flow);
        fs.push = 0.8f;
    }
    return fs;
}

void ephysics_fluid_apply(world *w, ephys_body *b, float dt) {
    b->flags &= ~(uint32_t)(EPHYS_F_IN_WATER | EPHYS_F_SUBMERGED);
    b->fluid_h = 0.0f;

    ephys_fluid_state fs = ephysics_fluid_sample(w, b);
    if (fs.density <= 0.0f) return;

    float frac = submerged_fraction(b, fs.surface_y);
    if (frac <= 0.0f) return;

    b->flags |= EPHYS_F_IN_WATER;
    b->fluid_h = (fs.surface_y - ephysics_body_box(b).min.y);

    // eye point (top of box) under the surface => fully submerged for control
    float eye_y = b->pos.y + b->half.y * 2.0f - 0.15f;
    if (eye_y < fs.surface_y) b->flags |= EPHYS_F_SUBMERGED;

    // buoyancy: archimedes-ish. displaced volume scales with submerged frac.
    // net vertical accel = (buoyancy*density - 1) * |g|, scaled by how deep.
    float net = (b->mat.buoyancy * fs.density - 1.0f);
    float a   = net * fabsf(GRAVITY) * frac;
    b->vel.y += a * dt;

    // drag. heavier than air, applied as exponential decay toward zero so it's
    // framerate-ish stable. only the submerged fraction drags.
    float drag = 1.0f - (1.0f - b->mat.water_drag) * frac;
    b->vel.x *= drag;
    b->vel.z *= drag;
    b->vel.y *= (drag + 1.0f) * 0.5f;   // less vertical drag so floating works

    // current push
    if (fs.push > 0.0f) {
        b->vel.x += fs.flow.x * fs.push * frac * dt;
        b->vel.z += fs.flow.z * fs.push * frac * dt;
    }
}
