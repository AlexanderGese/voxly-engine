#ifndef RENDER_LODTERRAIN_LT_SURFACE_H
#define RENDER_LODTERRAIN_LT_SURFACE_H

#include "lt_types.h"
#include "lt_grid.h"
#include "lt_mesh.h"

// the coarse surface pass. walks every solid cell in the grid and emits a face
// for each side that borders air (or the grid edge). no greedy merging here on
// purpose — at lod cell counts the win is tiny and the seam code is much easier
// to reason about when every face is exactly one cell wide.

// emit all exposed faces of `g` into `m`. `src` is used only for the light
// sample at each face so far chunks still respond to time-of-day. cells are
// scaled by step into block-space, then offset by the source base so the mesh
// lands at the chunk's true world position.
void lt_surface_build(lt_mesh *m, const lt_grid *g, const lt_source *src);

// emit a single face of cell (x,y,z) in direction `face`. broken out so the
// seam pass can re-emit a border face at a stitched position. `tile` is the
// atlas id, `light` the 0..15 face light. world_origin is base_x/base_z folded
// in plus the cell->block scale already applied by the caller.
void lt_surface_face(lt_mesh *m, int face,
                     float bx, float by, float bz, float cell,
                     int tile, int light, int is_skirt);

#endif
