#include "mineshaft_cobweb.h"
#include "mineshaft_rand.h"

// a corner is "weby" if it hugs a vertical edge of the cell and sits high up. we
// weight spawn probability by how cornered/high a block is so webs cluster where
// they belong instead of speckling the whole room evenly.
static float corner_weight(mineshaft_box box, int x, int y, int z,
                           int floor_y, int ceil_y) {
    int dx0 = x - box.x0, dx1 = box.x1 - 1 - x;
    int dz0 = z - box.z0, dz1 = box.z1 - 1 - z;
    int edx = dx0 < dx1 ? dx0 : dx1;        // distance to nearest x wall
    int edz = dz0 < dz1 ? dz0 : dz1;        // distance to nearest z wall
    int wall_d = edx < edz ? edx : edz;

    float w = 1.0f - (float)wall_d * 0.35f;
    if (w < 0.05f) w = 0.05f;

    int span = ceil_y - floor_y;
    if (span > 0) {
        float up = (float)(y - floor_y) / (float)span;
        w *= 0.4f + up * 0.6f;              // ceiling bias
    }
    return w;
}

int mineshaft_cobweb_fill(mineshaft_buffer *b, const mineshaft_config *cfg,
                          mineshaft_box cell_box, int floor_y, int ceil_y,
                          float density, uint32_t seed) {
    if (density <= 0.0f) return 0;
    int n = 0;
    // interior only - never overwrite the wall shell or the floor.
    for (int y = floor_y + 1; y < ceil_y; y++) {
        for (int z = cell_box.z0 + 1; z < cell_box.z1 - 1; z++) {
            for (int x = cell_box.x0 + 1; x < cell_box.x1 - 1; x++) {
                float w = corner_weight(cell_box, x, y, z, floor_y, ceil_y);
                float p = density * w;
                // stable per-block roll; fold seed so adjacent shafts differ.
                float r = mineshaft_hash_f01(x, y, z, seed ^ 0x57eb7000u);
                if (r < p)
                    n += mineshaft_buffer_add(b, x, y, z, cfg->mat_web);
            }
        }
    }
    return n;
}

int mineshaft_cobweb_curtain(mineshaft_buffer *b, const mineshaft_config *cfg,
                             int x, int y0, int y1, int z,
                             int axis, int len, float density, uint32_t seed) {
    int n = 0;
    int step = len >= 0 ? 1 : -1;
    int cnt  = len >= 0 ? len : -len;
    for (int i = 0; i < cnt; i++) {
        int px = x, pz = z;
        if (axis == 0) px += i * step;
        else           pz += i * step;
        for (int y = y0; y < y1; y++) {
            float r = mineshaft_hash_f01(px, y, pz, seed ^ 0xc0bbeb00u);
            if (r < density)
                n += mineshaft_buffer_add(b, px, y, pz, cfg->mat_web);
        }
    }
    return n;
}
