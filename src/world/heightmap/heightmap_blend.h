#ifndef WORLD_HEIGHTMAP_BLEND_H
#define WORLD_HEIGHTMAP_BLEND_H

#include "heightmap_params.h"
#include "heightmap_field.h"

// cheap-sampling layer for the heightmap. resolving every column through the
// full spline pipeline is fine for one chunk but murder if you want a height
// for, say, every block of a 256-wide structure footprint. so we resolve on a
// coarse lattice and bilinearly interpolate the inside cells. the splines are
// smooth enough that a 4-block lattice is invisible for most terrain; cliffs
// blur a touch but nobody has complained.
//
// also lives here: the cross-chunk seam smoother. each chunk resolves its own
// field independently, which is deterministic so the seams already line up,
// but the soil-depth and steepness derived values can disagree by a block at
// the border. this nudges them into agreement using the skirt the field
// already carries.

// lattice step in blocks. must divide the patch cleanly. 4 is the sweet spot.
#define HEIGHTMAP_LATTICE  4

// a coarse height patch over a region, resolved on the lattice and ready to
// interpolate. the region is square, side = (cells-1)*lattice blocks.
typedef struct {
    int   ox, oz;        // world coord of corner sample [0][0]
    int   cells;         // samples per side, incl. both ends
    int   step;          // lattice step in blocks
    int   h[33 * 33];    // coarse heights, row-major. max 32-step lattice.
} heightmap_patch;

// resolve a coarse patch covering [ox,ox+side] x [oz,oz+side] where
// side = (cells-1)*step. cells is clamped to fit the buffer.
void heightmap_blend_patch(heightmap_patch *patch, const heightmap_params *p,
                           int ox, int oz, int cells, int step);

// bilinearly sample the patch at an arbitrary world column inside its region.
// clamps to the patch edges if the column is just outside.
float heightmap_blend_sample(const heightmap_patch *patch, int wx, int wz);

// smooth a field's steepness/underwater border against itself using the skirt
// so adjacent chunks agree on the edge classification. in place.
void heightmap_blend_seams(heightmap_field *f);

#endif
