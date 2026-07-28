#include "ephysics_current.h"
#include "ephysics_fluid.h"
#include "ephysics_aabb.h"

#include <math.h>

// strongest push a full-flow cell imparts, m/s. tuned so a 3-wide stream nudges
// you along without yanking. capped again on apply.
#define EPHYS_CURRENT_MAX 1.4f

// estimate a flow direction at a fluid cell. real fluid sim tracks levels, but
// from a body's point of view the useful signal is "which way does the water
// open up": a fluid cell with a non-fluid (air) horizontal neighbour flows
// toward that neighbour, and downhill if there's fluid below open air. we sum
// contributions from the 4 horizontal neighbours.
static vec3 cell_flow(world *w, int x, int y, int z) {
    vec3 f = VEC3_ZERO;
    const int nx[4] = {  1, -1,  0,  0 };
    const int nz[4] = {  0,  0,  1, -1 };

    for (int i = 0; i < 4; i++) {
        int ax = x + nx[i], az = z + nz[i];
        block_id side = world_get_block(w, ax, y, az);
        if (ephysics_is_fluid(side)) continue;       // same medium, no gradient

        // a non-solid (air) opening pulls flow toward it; a solid wall doesnt.
        if (!block_is_solid(side)) {
            f.x += (float)nx[i];
            f.z += (float)nz[i];
        }
    }

    // a waterfall: open drop below with no side openings still drags you toward
    // the lip, so nudge along whatever side direction we already leaned.
    block_id below = world_get_block(w, x, y - 1, z);
    if (!ephysics_is_fluid(below) && !block_is_solid(below)) {
        f.x *= 1.3f;
        f.z *= 1.3f;
    }
    return f;
}

ephys_current ephysics_current_sample(world *w, const ephys_body *b) {
    ephys_current out;
    out.dir = VEC3_ZERO;
    out.strength = 0.0f;
    out.submersion = 0.0f;

    aabb box = ephysics_body_box(b);

    int x0 = (int)floorf(box.min.x), x1 = (int)floorf(box.max.x);
    int y0 = (int)floorf(box.min.y), y1 = (int)floorf(box.max.y);
    int z0 = (int)floorf(box.min.z), z1 = (int)floorf(box.max.z);

    vec3 accum = VEC3_ZERO;
    int fluid_cells = 0, total_cells = 0;

    for (int y = y0; y <= y1; y++)
        for (int z = z0; z <= z1; z++)
            for (int x = x0; x <= x1; x++) {
                total_cells++;
                block_id id = world_get_block(w, x, y, z);
                if (!ephysics_is_fluid(id)) continue;
                fluid_cells++;
                accum = vec3_add(accum, cell_flow(w, x, y, z));
            }

    if (fluid_cells == 0 || total_cells == 0) return out;

    out.submersion = (float)fluid_cells / (float)total_cells;

    float len = vec3_length(accum);
    if (len < 1e-4f) return out;   // submerged but in still water

    out.dir = vec3_scale(accum, 1.0f / len);
    // strength scales with how consistent the flow was (len grows when the
    // neighbour openings all agree) but saturates fast.
    float consist = len / (float)fluid_cells;
    if (consist > 1.0f) consist = 1.0f;
    out.strength = EPHYS_CURRENT_MAX * consist;
    return out;
}

void ephysics_current_apply(ephys_body *b, const ephys_current *cur, float dt) {
    if (cur->strength <= 0.0f) return;

    float push = cur->strength * cur->submersion * dt * 20.0f;  // dt*~tickrate
    if (push > EPHYS_CURRENT_MAX) push = EPHYS_CURRENT_MAX;

    b->vel.x += cur->dir.x * push;
    b->vel.z += cur->dir.z * push;

    // dont let the current alone overspeed the body past the sweep's safe cap.
    float sp2 = b->vel.x * b->vel.x + b->vel.z * b->vel.z;
    float cap = b->mat.max_speed + EPHYS_CURRENT_MAX;
    if (sp2 > cap * cap) {
        float s = cap / sqrtf(sp2);
        b->vel.x *= s;
        b->vel.z *= s;
    }
}
