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
