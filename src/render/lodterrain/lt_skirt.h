#ifndef RENDER_LODTERRAIN_LT_SKIRT_H
#define RENDER_LODTERRAIN_LT_SKIRT_H

#include "lt_types.h"
#include "lt_grid.h"
#include "lt_mesh.h"
#include "lt_seam.h"

// the skirt pass. for every border cell on a seamed face it drops a vertical
// quad of LT_SKIRT_DEPTH blocks straight down from the cell's outer edge. that
// curtain of geometry plugs the gap the coarser neighbour leaves, so you don't
// see through the world at chunk boundaries. it's a couple extra quads per
// border cell and nobody ever notices them — that's the point.

// append skirts for all flagged borders into `m`. `g` is the meshed grid, `src`
// positions it, `mask` says which faces need it. returns the number of skirt
// quads added.
int lt_skirt_build(lt_mesh *m, const lt_grid *g, const lt_source *src,
                   lt_seam_mask mask);

// emit one skirt quad hanging down from the outer edge of cell (x,y,z) on
// horizontal face `face`. depth is in blocks. broken out for the targeted
// re-stitch path and the test.
void lt_skirt_edge(lt_mesh *m, int face,
                   float bx, float by, float bz, float cell, float depth,
                   int tile, int light);

#endif
