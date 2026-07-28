#include "ephysics_current.h"
#include "ephysics_fluid.h"
#include "ephysics_aabb.h"
#include <math.h>
#define EPHYS_CURRENT_MAX 1.4f
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
for (int y = y0;
y <= y1;
y++)
        for (int z = z0;
z <= z1;
z++)
            for (int x = x0;
x <= x1;
out.submersion = (float)fluid_cells / (float)total_cells;
float len = vec3_length(accum);
if (len < 1e-4f) return out;
out.dir = vec3_scale(accum, 1.0f / len);
float consist = len / (float)fluid_cells;
if (consist > 1.0f) consist = 1.0f;
out.strength = EPHYS_CURRENT_MAX * consist;
return out;
