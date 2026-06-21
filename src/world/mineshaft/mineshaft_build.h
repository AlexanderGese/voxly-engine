#ifndef WORLD_MINESHAFT_BUILD_H
#define WORLD_MINESHAFT_BUILD_H

#include "mineshaft_buffer.h"
#include "mineshaft_grid.h"
#include "mineshaft_rand.h"
#include "mineshaft_types.h"

// the stamping pass. walks the classified grid and turns every non-empty cell
// into voxels: digs the passage, knocks doorways toward linked neighbours, then
// dispatches per cell kind to add supports / ore / ladders. cobwebs go on last
// so they land in freshly-carved air. cell footprint pitch is cfg->corridor_len.

// corridor headroom, in blocks (floor exclusive). a local constant rather than a
// config field: it's tied to the doorway/support math, not a host-facing knob.
#define MINESHAFT_CORRIDOR_H   3

// side length of the square maze grid. derived to stay under cfg->max_pieces and
// the static grid cap. one place so place/build/driver agree on extent.
int mineshaft_build_grid_dim(const mineshaft_config *cfg);

// world-space footprint box of a grid cell. exposed so the driver can test which
// chunks a shaft touches before committing to building it.
mineshaft_box mineshaft_build_cell_box(const mineshaft_site *site,
                                       const mineshaft_config *cfg,
                                       int gdim, int cx, int cz);

// build one cell into the buffer. returns voxels emitted.
int mineshaft_build_cell(mineshaft_buffer *b, mineshaft_grid *g,
                         const mineshaft_site *site, const mineshaft_config *cfg,
                         int cx, int cz, mineshaft_rng *rng);

// build the whole grid. returns total voxels, bailing at cfg->max_voxels.
int mineshaft_build_grid(mineshaft_buffer *b, mineshaft_grid *g,
                         const mineshaft_site *site, const mineshaft_config *cfg,
                         mineshaft_rng *rng);

#endif
