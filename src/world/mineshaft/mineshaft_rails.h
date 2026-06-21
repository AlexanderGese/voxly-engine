#ifndef WORLD_MINESHAFT_RAILS_H
#define WORLD_MINESHAFT_RAILS_H

#include "mineshaft_buffer.h"
#include "mineshaft_box.h"
#include "mineshaft_grid.h"
#include "mineshaft_rand.h"
#include "mineshaft_types.h"

// minecart rails. the spine of a working corridor used to carry track; an
// abandoned one has it in patches - rusted out in the middle, intact near the
// junctions where it was reinforced. there's no dedicated rail block id, so we
// reuse cfg->mat_rail (a wood slab stand-in) laid flat on the floor, with the
// occasional minecart marker. rails follow the cell's link axis so a straight
// run reads continuous across cell borders.

// lay a rail run down the floor of a corridor cell along its passage axis. gaps
// are punched where the track has rotted (governed by `wear`, [0,1]). returns
// voxels added. straight cells only - junctions/rooms skip rails.
int mineshaft_rails_cell(mineshaft_buffer *b, const mineshaft_config *cfg,
                         const mineshaft_grid *g, int cx, int cz,
                         mineshaft_box cell_box, int floor_y, float wear,
                         mineshaft_rng *rng);

// drop a single derelict minecart marker on the rail at (x,z). cheap flavour;
// the caller decides when one's earned (low odds, near dead ends). returns
// voxels added (1 if placed, 0 if it rolled a miss).
int mineshaft_rails_minecart(mineshaft_buffer *b, const mineshaft_config *cfg,
                             int x, int floor_y, int z, mineshaft_rng *rng);

// is this cell a straight two-ended corridor (exactly one passage axis)? rails
// only make sense on those; corners and tees get skipped. exposed for the build
// pass to gate the rails call.
int mineshaft_rails_is_straight(const mineshaft_grid *g, int cx, int cz);

#endif
