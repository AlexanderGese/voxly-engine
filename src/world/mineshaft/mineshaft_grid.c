#include "mineshaft_grid.h"
#include "mineshaft_box.h"
#include <string.h>

// shared zero cell handed out for out-of-range reads. callers only ever read
// .kind/.links off it (it stays EMPTY/0), so a single static is safe enough.
static mineshaft_cell g_void_cell;

void mineshaft_grid_init(mineshaft_grid *g, int w, int d) {
    if (w < 1) w = 1;
    if (d < 1) d = 1;
    if (w > MINESHAFT_GRID_MAX) w = MINESHAFT_GRID_MAX;
    if (d > MINESHAFT_GRID_MAX) d = MINESHAFT_GRID_MAX;
    g->w = w;
    g->d = d;
    g->entry_x = w / 2;
    g->entry_z = d / 2;
    memset(g->cells, 0, sizeof g->cells);
}

int mineshaft_grid_in_bounds(const mineshaft_grid *g, int x, int z) {
    return x >= 0 && x < g->w && z >= 0 && z < g->d;
}

mineshaft_cell *mineshaft_grid_at(mineshaft_grid *g, int x, int z) {
    if (!mineshaft_grid_in_bounds(g, x, z)) {
        // hand back a fresh zero cell each time; callers must not write it.
        g_void_cell.kind = MS_CELL_EMPTY;
        g_void_cell.links = 0;
        g_void_cell.depth = 0;
        g_void_cell.flags = 0;
        return &g_void_cell;
    }
    return &g->cells[z * g->w + x];
}

int mineshaft_grid_degree(const mineshaft_grid *g, int x, int z) {
    if (!mineshaft_grid_in_bounds(g, x, z)) return 0;
    uint8_t l = g->cells[z * g->w + x].links;
    int n = 0;
    n += (l & MS_LINK_N) != 0;
    n += (l & MS_LINK_E) != 0;
    n += (l & MS_LINK_S) != 0;
    n += (l & MS_LINK_W) != 0;
    return n;
}

void mineshaft_grid_link(mineshaft_grid *g, int x, int z, mineshaft_dir d) {
    if (!mineshaft_grid_in_bounds(g, x, z)) return;
    int dx, dz;
    mineshaft_dir_step(d, &dx, &dz);
    int nx = x + dx, nz = z + dz;
    if (!mineshaft_grid_in_bounds(g, nx, nz)) return;

    g->cells[z * g->w + x].links   |= (uint8_t)mineshaft_dir_link_bit(d);
    mineshaft_dir back = mineshaft_dir_opposite(d);
    g->cells[nz * g->w + nx].links |= (uint8_t)mineshaft_dir_link_bit(back);
}
