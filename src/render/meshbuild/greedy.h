#ifndef RENDER_MESHBUILD_GREEDY_H
#define RENDER_MESHBUILD_GREEDY_H

// greedy meshing. for one face direction we sweep slice by slice along the
// sweep axis, fill a mask of visible faces, then merge the mask into the
// largest possible axis-aligned rectangles. classic two-phase greedy mesh.
//
// this is the bit that actually cuts the triangle count — a flat stone floor
// goes from thousands of quads to one. the mask building also folds in ao and
// light so merges respect shading.

#include "mb_types.h"
#include "voxel_mask.h"

// where a finished quad should land. greedy meshing hands each merged quad to
// this so the caller can route opaque vs translucent into separate buffers
// (see pass_split). `block` is the source block so the sink can pick a bucket.
typedef void (*mb_emit_fn)(void *user, const mb_quad *q,
                           block_id block, int base_x, int base_z);

// run greedy meshing for a single sweep axis (0=x,1=y,2=z), both its + and -
// faces. merged quads are handed to `emit`. `scratch` is a caller-owned mask
// reused across axes so we dont keep a 2k-cell struct on the stack per call.
void mb_greedy_axis(const mb_ctx *c, int axis, mb_mask *scratch,
                    mb_emit_fn emit, void *user);

#endif
