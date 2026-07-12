#include "lt_sample.h"

#include "../../world/block.h"
#include "../../config.h"
#include <string.h>

// majority vote over a step^3 box. BLOCK_COUNT is small (16ish) so a flat tally
// array is faster than any hashmap nonsense and fits in a cache line.
block_id lt_sample_box(const lt_source *src, int step,
                       int base_x, int base_z,
                       int cx, int cy, int cz) {
    int tally[BLOCK_COUNT];
    memset(tally, 0, sizeof(tally));

    int ox = base_x + cx * step;
    int oy = cy * step;
    int oz = base_z + cz * step;

    int sub_total = 0;   // sub-voxels actually inside the chunk's y range
    int solid     = 0;   // of those, how many were opaque
    block_id best = BLOCK_AIR;
    int best_n = 0;

    for (int dy = 0; dy < step; dy++) {
        int wy = oy + dy;
        if (wy < 0 || wy >= CHUNK_SIZE_Y) continue;   // clamp the tall axis
        for (int dz = 0; dz < step; dz++) {
            for (int dx = 0; dx < step; dx++) {
                block_id id = src->sample(src->ctx, ox + dx, wy, oz + dz);
                sub_total++;
                if (id == BLOCK_AIR || !block_is_opaque(id)) continue;
                solid++;
                int n = ++tally[id];
                if (n > best_n) { best_n = n; best = id; }
            }
        }
    }

    if (sub_total == 0) return BLOCK_AIR;

    // threshold is against the count of cells that could have been solid, so a
    // box hanging off the top of the world (mostly empty) isn't penalised for
    // the air above it — those don't count toward sub_total.
    float frac = (float)solid / (float)sub_total;
    if (frac < LT_SOLID_THRESHOLD) return BLOCK_AIR;
    return best;
}

int lt_sample_grid(lt_grid *g, const lt_source *src) {
    int step = g->step;
    int solid_cells = 0;

    for (int y = 0; y < g->ny; y++) {
        for (int z = 0; z < g->nz; z++) {
            for (int x = 0; x < g->nx; x++) {
                block_id id = lt_sample_box(src, step,
                                            src->base_x, src->base_z,
                                            x, y, z);
                lt_grid_set(g, x, y, z, id);
                if (id != BLOCK_AIR) solid_cells++;
            }
        }
    }
    return solid_cells;
}
