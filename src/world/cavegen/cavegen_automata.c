#include "cavegen_automata.h"

#include "cavegen_noise.h"
#include "../../config.h"
#include <stdlib.h>
#include <string.h>

// whether a cell is even allowed to become a cave: under the surface crust,
// over the floor, and where the cheese noise dips below threshold.
static int cell_eligible(const cavegen_grid *g, const cavegen_params *p,
                         int x, int y, int z) {
    if (y < p->min_y) return 0;
    int surf = cavegen_grid_height(g, x, z);
    if (y > surf - p->surface_margin) return 0;

    int wx, wy, wz;
    cavegen_cell_to_world(g, x, y, z, &wx, &wy, &wz);
    float n = cavegen_fbm3((float)wx * p->cheese_scale,
                           (float)wy * p->cheese_scale,
                           (float)wz * p->cheese_scale,
                           p->seed ^ 0xca5e0001u, 3, 2.0f, 0.5f);
    return n < p->cheese_thresh;
}

void cavegen_automata_seed(cavegen_grid *g, const cavegen_params *p) {
    for (int y = 0; y < CAVEGEN_DIM_Y; y++) {
        for (int z = 0; z < CAVEGEN_DIM_Z; z++) {
            for (int x = 0; x < CAVEGEN_DIM_X; x++) {
                if (!cell_eligible(g, p, x, y, z)) {
                    cavegen_grid_set(g, x, y, z, CAVEGEN_SOLID);
                    continue;
                }
                int wx, wy, wz;
                cavegen_cell_to_world(g, x, y, z, &wx, &wy, &wz);
                float r = cavegen_hash_f01(wx, wy, wz, p->seed ^ 0xf111u);
                // r below fill_chance stays solid, above opens. so a higher
                // fill_chance means denser rock, which is the natural reading.
                cavegen_grid_set(g, x, y, z,
                                 r < p->fill_chance ? CAVEGEN_SOLID : CAVEGEN_AIR);
            }
        }
    }
}

int cavegen_automata_solid_neighbours(const cavegen_grid *g, int x, int y, int z) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0 && dz == 0) continue;
                int nx = x + dx, ny = y + dy, nz = z + dz;
                // oob -> solid. keeps caves from leaking out the skirt.
                if (!cavegen_grid_in_bounds(nx, ny, nz)) { count++; continue; }
                if (!cavegen_cell_is_open(cavegen_grid_get(g, nx, ny, nz)))
                    count++;
            }
        }
    }
    return count;
}

void cavegen_automata_smooth(cavegen_grid *g, const cavegen_params *p) {
    uint8_t *next = malloc(CAVEGEN_CELLS);
    if (!next) return;

    for (int it = 0; it < p->ca_iterations; it++) {
        memcpy(next, g->cells, CAVEGEN_CELLS);

        for (int y = 0; y < CAVEGEN_DIM_Y; y++) {
            for (int z = 0; z < CAVEGEN_DIM_Z; z++) {
                for (int x = 0; x < CAVEGEN_DIM_X; x++) {
                    // only churn cells that were eligible to begin with, so the
                    // crust and noise mask stay respected across iterations.
                    if (!cell_eligible(g, p, x, y, z)) continue;

                    int n = cavegen_automata_solid_neighbours(g, x, y, z);
                    uint8_t cur = cavegen_grid_get(g, x, y, z);
                    int idx = cavegen_grid_idx(x, y, z);

                    if (cavegen_cell_is_open(cur)) {
                        // open cell becomes solid if too crowded (birth rule)
                        next[idx] = (n >= p->ca_birth) ? CAVEGEN_SOLID
                                                       : CAVEGEN_AIR;
                    } else {
                        // solid cell survives if it has enough solid neighbours
                        next[idx] = (n >= p->ca_survive) ? CAVEGEN_SOLID
                                                         : CAVEGEN_AIR;
                    }
                }
            }
        }
        memcpy(g->cells, next, CAVEGEN_CELLS);
    }
    free(next);
}
