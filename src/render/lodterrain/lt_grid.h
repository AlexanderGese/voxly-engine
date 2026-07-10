#ifndef RENDER_LODTERRAIN_LT_GRID_H
#define RENDER_LODTERRAIN_LT_GRID_H

#include "lt_types.h"

// the coarse voxel grid. one lives per (chunk, level) while a build is in
// flight; it gets freed once the mesh pops out. allocation is a flat block_id
// array so it stays cache-friendly when the surface pass scans it.

// allocate a grid sized for `level`. cell counts are the full-res chunk dims
// divided by the level's step, rounded up so a 16-wide chunk at step 8 still
// gets 2 cells (not 1.x). cells start zeroed = BLOCK_AIR. returns 0 on oom.
int  lt_grid_init(lt_grid *g, int level);

// release the cell buffer. safe to call on a zeroed grid.
void lt_grid_free(lt_grid *g);

// linear index for a cell. no bounds check in release; the surface scan stays
// inside [0,n) by construction so this is hot-path lean.
static inline int lt_grid_idx(const lt_grid *g, int x, int y, int z) {
    return x + z * g->nx + y * g->nx * g->nz;
}

// bounds-checked read. out-of-range reads as air so the mesher can treat the
// chunk border uniformly without special-casing the edges.
static inline block_id lt_grid_get(const lt_grid *g, int x, int y, int z) {
    if (x < 0 || x >= g->nx) return BLOCK_AIR;
    if (y < 0 || y >= g->ny) return BLOCK_AIR;
    if (z < 0 || z >= g->nz) return BLOCK_AIR;
    return g->cells[lt_grid_idx(g, x, y, z)];
}

static inline void lt_grid_set(lt_grid *g, int x, int y, int z, block_id id) {
    g->cells[lt_grid_idx(g, x, y, z)] = id;
}

// is a cell solid-ish? air and non-opaque (glass, water) don't occlude their
// neighbours at this distance so we treat only opaque blocks as fill. keeps the
// far silhouette clean.
int  lt_grid_cell_solid(const lt_grid *g, int x, int y, int z);

// total cell count, handy for stats and zeroing.
static inline int lt_grid_volume(const lt_grid *g) {
    return g->nx * g->ny * g->nz;
}

#endif
