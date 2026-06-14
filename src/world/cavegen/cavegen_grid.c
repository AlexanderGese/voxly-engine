#include "cavegen_grid.h"

#include "../../config.h"
#include "../../util/log.h"
#include <stdlib.h>
#include <string.h>

cavegen_grid *cavegen_grid_create(void) {
    cavegen_grid *g = calloc(1, sizeof *g);
    if (!g) return NULL;
    g->cells = malloc(CAVEGEN_CELLS);
    if (!g->cells) {
        LOGE("cavegen: grid alloc failed (%d cells)", CAVEGEN_CELLS);
        free(g);
        return NULL;
    }
    return g;
}

void cavegen_grid_destroy(cavegen_grid *g) {
    if (!g) return;
    free(g->cells);
    free(g);
}

void cavegen_grid_reset(cavegen_grid *g, cavegen_origin origin) {
    memset(g->cells, CAVEGEN_SOLID, CAVEGEN_CELLS);
    g->origin = origin;
    // heightmap left stale on purpose, the column fill rewrites it every region.
    for (int i = 0; i < CAVEGEN_DIM_X * CAVEGEN_DIM_Z; i++)
        g->heightmap[i] = CHUNK_SIZE_Y - 1;
}

int cavegen_grid_idx(int x, int y, int z) {
    // x fastest, then z, then y. same ordering vibe as chunk_idx but our own dims.
    return x + z * CAVEGEN_DIM_X + y * CAVEGEN_DIM_X * CAVEGEN_DIM_Z;
}

int cavegen_grid_in_bounds(int x, int y, int z) {
    return x >= 0 && x < CAVEGEN_DIM_X &&
           y >= 0 && y < CAVEGEN_DIM_Y &&
           z >= 0 && z < CAVEGEN_DIM_Z;
}

uint8_t cavegen_grid_get(const cavegen_grid *g, int x, int y, int z) {
    if (!cavegen_grid_in_bounds(x, y, z)) return CAVEGEN_SOLID; // oob reads solid
    return g->cells[cavegen_grid_idx(x, y, z)];
}

void cavegen_grid_set(cavegen_grid *g, int x, int y, int z, uint8_t v) {
    if (!cavegen_grid_in_bounds(x, y, z)) return;
    g->cells[cavegen_grid_idx(x, y, z)] = v;
}

int cavegen_cell_is_open(uint8_t v) {
    return v == CAVEGEN_AIR || v == CAVEGEN_WORM || v == CAVEGEN_FLOODED;
}

void cavegen_grid_set_height(cavegen_grid *g, int x, int z, int y) {
    if (x < 0 || x >= CAVEGEN_DIM_X || z < 0 || z >= CAVEGEN_DIM_Z) return;
    g->heightmap[x + z * CAVEGEN_DIM_X] = y;
}

int cavegen_grid_height(const cavegen_grid *g, int x, int z) {
    if (x < 0 || x >= CAVEGEN_DIM_X || z < 0 || z >= CAVEGEN_DIM_Z)
        return CHUNK_SIZE_Y - 1;
    return g->heightmap[x + z * CAVEGEN_DIM_X];
}

void cavegen_cell_to_world(const cavegen_grid *g, int x, int y, int z,
                           int *wx, int *wy, int *wz) {
    // cell (PAD,_,PAD) is the chunk's local (0,_,0), so world coord is
    // base + (cell - pad). base already includes the chunk's world origin.
    if (wx) *wx = g->origin.base_x + (x - CAVEGEN_PAD);
    if (wy) *wy = y;
    if (wz) *wz = g->origin.base_z + (z - CAVEGEN_PAD);
}

int cavegen_grid_count(const cavegen_grid *g, uint8_t state) {
    int n = 0;
    for (int i = 0; i < CAVEGEN_CELLS; i++)
        if (g->cells[i] == state) n++;
    return n;
}
