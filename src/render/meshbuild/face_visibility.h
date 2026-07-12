#ifndef RENDER_MESHBUILD_FACE_VISIBILITY_H
#define RENDER_MESHBUILD_FACE_VISIBILITY_H
// decides whether a face between two cells should produce geometry. this is
// the same logic the naive mesher inlines, pulled out so greedy meshing and
// the per-face fallback share one source of truth.
#include "mb_types.h"
// is `here` a block that draws faces at all? air and (later) other "no mesh"
// blocks return 0.
int mb_face_renderable(block_id here);
// should we emit a face for `here` against neighbour `there`?
// - skip if `here` is air
// - skip if the neighbour is opaque (hidden)
// - skip if both are the same transparent block (merge interiors of glass,
// leaves, water — otherwise you get z-fighting internal walls)
// returns 1 when a face is needed.
int mb_face_visible(block_id here, block_id there);
// two faces can be greedy-merged only when they are literally the same kind of
// surface: same block, same tile for this face, same neighbour-transparency
// situation. ao/light equality is checked separately by the greedy pass.
int mb_face_mergeable(block_id a, int tile_a, block_id b, int tile_b);
#endif
