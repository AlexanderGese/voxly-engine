#ifndef RENDER_LODTERRAIN_LT_CONFIG_H
#define RENDER_LODTERRAIN_LT_CONFIG_H

// lod terrain tunables. these live apart from the global config.h so you can
// poke at terrain lod without recompiling the universe. magic numbers tuned by
// eye against RENDER_DISTANCE=6 — degrades ok if you crank it.

#include "../../config.h"

// number of discrete lod levels. level 0 = full res (1 voxel = 1 block),
// each subsequent level halves the linear resolution. with 4 levels a chunk
// goes 16 -> 8 -> 4 -> 2 cells on the horizontal axes.
#define LT_LEVEL_COUNT      4

// the linear downsample factor for a level is 1<<level. level 0 = 1, the
// finest, level 3 = 8. y is downsampled the same way.
#define LT_STEP(level)      (1 << (level))

// the grid is allocated for the finest level and reused; this is the cell
// count of the full-res grid per axis. matches chunk dims.
#define LT_GRID_X           CHUNK_SIZE_X
#define LT_GRID_Y           CHUNK_SIZE_Y
#define LT_GRID_Z           CHUNK_SIZE_Z

// how many solid sub-cells (out of step^3) a coarse cell needs before it
// counts as solid. half-ish, so a coarse cell appears when its volume is
// mostly filled. keeps far hills from looking like swiss cheese.
#define LT_SOLID_THRESHOLD  0.45f

// seam skirts: when a fine chunk borders a coarser one we drop a vertical
// skirt down this many blocks along the shared edge to hide the gap that
// t-junctions would otherwise leave. cheap and it just works.
#define LT_SKIRT_DEPTH      3.0f

// distance hysteresis as a fraction of band width. stops chunks ping-ponging
// between two levels when you stand right on a ring boundary.
#define LT_HYSTERESIS       0.10f

// initial vertex/index reserve for a fresh build buffer. most coarse meshes
// fit; the darray grows if not.
#define LT_VERT_RESERVE     512
#define LT_IDX_RESERVE      768

#endif
