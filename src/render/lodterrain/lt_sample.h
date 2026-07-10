#ifndef RENDER_LODTERRAIN_LT_SAMPLE_H
#define RENDER_LODTERRAIN_LT_SAMPLE_H

#include "lt_types.h"
#include "lt_grid.h"

// downsampling: read the full-res world through the source callbacks and fold
// each step^3 box of blocks into a single coarse cell. the cell takes the most
// common opaque block in the box (its "dominant" id) but only if enough of the
// box was solid — otherwise the cell is air. that majority vote is what keeps a
// distant grass hill reading as grass and not a speckled mess.

// fill an already-init'd grid `g` from the world via `src`. `g` must have been
// lt_grid_init'd at the level you want. returns the number of solid cells, which
// the caller uses to early-out an all-air column before meshing.
int lt_sample_grid(lt_grid *g, const lt_source *src);

// dominant-block vote over one step^3 box anchored at coarse cell (cx,cy,cz).
// base_x/base_z come from the source. returns BLOCK_AIR if the box didn't pass
// LT_SOLID_THRESHOLD. exposed for the unit test that checks the vote.
block_id lt_sample_box(const lt_source *src, int step,
                       int base_x, int base_z,
                       int cx, int cy, int cz);

#endif
