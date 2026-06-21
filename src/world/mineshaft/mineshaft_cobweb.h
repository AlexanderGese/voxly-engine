#ifndef WORLD_MINESHAFT_COBWEB_H
#define WORLD_MINESHAFT_COBWEB_H

#include "mineshaft_buffer.h"
#include "mineshaft_box.h"
#include "mineshaft_types.h"

// cobweb scatter. abandoned shafts are full of them, clustered in corners and
// hanging off the beams. the web block is cfg->mat_web (a plant-class stand-in,
// see mineshaft_types.c). placement is driven by a positional hash so the
// pattern is stable across regen, with the seed only stirring the field.

// scatter webs through a carved cell box. `density` is the per-eligible-block
// probability; eligibility weights ceiling corners heaviest. returns voxels.
int mineshaft_cobweb_fill(mineshaft_buffer *b, const mineshaft_config *cfg,
                          mineshaft_box cell_box, int floor_y, int ceil_y,
                          float density, uint32_t seed);

// hang a denser web curtain across a passage opening - used at room mouths where
// nobody's walked in ages. spans the gap over the given y range and axis run.
int mineshaft_cobweb_curtain(mineshaft_buffer *b, const mineshaft_config *cfg,
                             int x, int y0, int y1, int z,
                             int axis, int len, float density, uint32_t seed);

#endif
