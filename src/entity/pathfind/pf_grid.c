#include "pf_grid.h"
#include "../../world/block.h"
#include <string.h>
#include <stdlib.h>
int pf_grid_walkable_at(world *w, int wx, int wy, int wz) {
    block_id feet  = world_get_block(w, wx, wy,     wz);
    block_id head  = world_get_block(w, wx, wy + 1, wz);
    block_id below = world_get_block(w, wx, wy - 1, wz);
    if (block_is_solid(feet) || block_is_solid(head)) return 0;
    return block_is_solid(below);
}

void pf_grid_init(pf_grid *g, world *w, pf_coord center) {
    g->w = w;
g->origin = pf_coord_make(center.x - PF_WINDOW_RADIUS,
                              center.y,
                              center.z - PF_WINDOW_RADIUS);
g->h = PF_MAX_STEP_UP + PF_MAX_STEP_DOWN + 2;
memset(g->cols, 0, sizeof g->cols);
}

int pf_grid_in_bounds(const pf_grid *g, int lx, int lz) {
    (void)g;
    return lx >= 0 && lx < PF_WINDOW_SIZE && lz >= 0 && lz < PF_WINDOW_SIZE;
}

int pf_grid_to_local(const pf_grid *g, pf_coord wc, int *lx, int *lz) {
    int x = wc.x - g->origin.x;
int z = wc.z - g->origin.z;
if (x < 0 || x >= PF_WINDOW_SIZE || z < 0 || z >= PF_WINDOW_SIZE) return 0;
*lx = x;
*lz = z;
return 1;
}

// look for a standable y in [near_y - down, near_y + up]. prefer the one
// closest to near_y so a flat path stays flat instead of hopping around.
int pf_grid_standable(pf_grid *g, int lx, int lz, int near_y, int *floor_out) {
    if (!pf_grid_in_bounds(g, lx, lz)) return 0;

    pf_column *col = &g->cols[lz * PF_WINDOW_SIZE + lx];
    // the cache is keyed on a single resolve height; if the query is for a
    // very different y we just re-resolve. cheap enough, columns are short.
    if (col->resolved && abs(col->floor_y - near_y) <= g->h) {
        if (!col->standable) return 0;
        *floor_out = col->floor_y;
        return 1;
    }

    int wx = g->origin.x + lx;
    int wz = g->origin.z + lz;

    int best = -9999;
    for (int dy = PF_MAX_STEP_UP; dy >= -PF_MAX_STEP_DOWN; dy--) {
        int wy = near_y + dy;
        if (pf_grid_walkable_at(g->w, wx, wy, wz)) {
            // closest-to-near_y wins; loop runs high->low so first hit
            // inside the up-range is fine, otherwise keep the nearest.
            if (best == -9999 || abs(wy - near_y) < abs(best - near_y))
                best = wy;
        }
    }

    col->resolved = 1;
    if (best == -9999) {
        col->standable = 0;
        return 0;
    }
    col->standable = 1;
    col->floor_y = (int16_t)best;
    *floor_out = best;
    return 1;
}
