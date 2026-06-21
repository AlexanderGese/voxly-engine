#ifndef WORLD_MINESHAFT_SUPPORT_H
#define WORLD_MINESHAFT_SUPPORT_H

#include "mineshaft_buffer.h"
#include "mineshaft_box.h"
#include "mineshaft_grid.h"
#include "mineshaft_rand.h"
#include "mineshaft_types.h"

// the iconic mineshaft support: two posts and a beam across the top, placed at a
// regular pitch down each corridor. abandoned ones are missing posts and have a
// rotted beam here and there, which is what sells the "old" look. materials come
// from cfg->mat_support / mat_beam so a host atlas can repalette without edits.
// runs after the corridor air has been carved.

// stamp one support frame: posts at (x0,z0) and (x1,z1) from floor_y up to
// ceil_y, with a beam at ceil_y joining them. `decay` in [0,1] is the chance any
// piece has rotted away. returns voxels written.
int mineshaft_support_frame(mineshaft_buffer *b, const mineshaft_config *cfg,
                            int x0, int z0, int x1, int z1,
                            int floor_y, int ceil_y, float decay,
                            mineshaft_rng *rng);

// line a whole corridor cell with frames at a fixed pitch. picks the support
// axis from the cell's link directions so frames straddle the passage.
int mineshaft_support_cell(mineshaft_buffer *b, const mineshaft_config *cfg,
                           const mineshaft_grid *g, int cx, int cz,
                           mineshaft_box cell_box,
                           int floor_y, int ceil_y, mineshaft_rng *rng);

// a junction gets a heavier crossing: a beam each axis plus a center post.
int mineshaft_support_junction(mineshaft_buffer *b, const mineshaft_config *cfg,
                               mineshaft_box cell_box,
                               int floor_y, int ceil_y, mineshaft_rng *rng);

#endif
