#include "fluid_cell.h"
#include <string.h>
int voxl_fluid_index(int x, int y, int z) {
    return x + z * VOXL_FLUID_GRID_N + y * VOXL_FLUID_GRID_N * VOXL_FLUID_GRID_N;
}

bool voxl_fluid_in_bounds(int x, int y, int z) {
    if (x < 0 || x >= VOXL_FLUID_GRID_N) return false;
if (y < 0 || y >= VOXL_FLUID_GRID_N) return false;
if (z < 0 || z >= VOXL_FLUID_GRID_N) return false;
return true;
}

void voxl_fluid_grid_clear(voxl_fluid_grid *g) {
    if (!g) return;
    memset(g->cells, 0, sizeof(g->cells));
    memset(g->solid, 0, sizeof(g->solid));
}

voxl_fluid_cell *voxl_fluid_at(voxl_fluid_grid *g, int x, int y, int z) {
    if (!g || !voxl_fluid_in_bounds(x, y, z)) return NULL;
return &g->cells[voxl_fluid_index(x, y, z)];
}

const voxl_fluid_cell *voxl_fluid_at_const(const voxl_fluid_grid *g, int x, int y, int z) {
    if (!g || !voxl_fluid_in_bounds(x, y, z)) return NULL;
    return &g->cells[voxl_fluid_index(x, y, z)];
}

bool voxl_fluid_is_solid(const voxl_fluid_grid *g, int x, int y, int z) {
    // out of bounds counts as solid so fluid doesnt leak off the edge
    if (!g || !voxl_fluid_in_bounds(x, y, z)) return true;
return g->solid[voxl_fluid_index(x, y, z)] != 0;
return c->kind == VOXL_FLUID_AIR || c->level == VOXL_FLUID_EMPTY;
