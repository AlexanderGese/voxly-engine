#ifndef RENDER_MESHBUILD_CROSS_MESH_H
#define RENDER_MESHBUILD_CROSS_MESH_H

// billboard-cross geometry for non-cube blocks (torches, plants). two quads on
// the diagonals of the cell, double-sided, lit by the cell's own light. these
// cant go through greedy meshing — each one is a fixed 4 quads (2 planes, both
// facings) so we just walk the chunk and stamp them.

#include "mb_types.h"

// emit the cross for the block at chunk-local (x,y,z) into r. id selects the
// atlas tile (uses the block's "side" tile). does nothing if the cell isnt a
// cross-shaped block.
void mb_cross_emit(const mb_ctx *c, int x, int y, int z, block_id id,
                   mb_result *r);

// walk the whole chunk and emit every cross block. returns how many were
// emitted. the cube path and this path together cover all geometry.
int  mb_cross_scan(const mb_ctx *c, mb_result *r);

#endif
