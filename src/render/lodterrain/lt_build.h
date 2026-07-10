#ifndef RENDER_LODTERRAIN_LT_BUILD_H
#define RENDER_LODTERRAIN_LT_BUILD_H

#include "lt_types.h"
#include "lt_mesh.h"
#include "lt_seam.h"

// the build orchestrator. one call turns "a chunk address + a level + its
// neighbour levels" into a finished cpu mesh, ready for lt_gpu to upload. it
// owns the temporary grid; the caller owns the resulting mesh.

// build the lod mesh for one chunk. `src` carries base_x/base_z and the sample
// callbacks. `level` is the lod to mesh at. `nb` gives the four neighbour levels
// for seam stitching (pass all-same level to skip skirts). the result is written
// into `out` which must already be lt_mesh_init'd; it is reset first.
//
// returns 1 on success, 0 if the grid allocation failed (out is left empty but
// valid). an all-air chunk succeeds with an empty mesh — that's not an error.
int lt_build_chunk(lt_mesh *out, const lt_source *src, int level,
                   const lt_neighbor_levels *nb);

// lighter variant with no seam handling, for the case where you know all
// neighbours share your level (the common interior-of-a-ring case).
int lt_build_chunk_simple(lt_mesh *out, const lt_source *src, int level);

#endif
