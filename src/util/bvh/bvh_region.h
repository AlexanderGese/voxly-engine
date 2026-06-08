#ifndef UTIL_BVH_REGION_H
#define UTIL_BVH_REGION_H
// world-facing glue. the bare bvh has no idea what a chunk or a block is, on
// purpose. but everyone wires it into chunk/block space, so the boilerplate for
// turning chunk coords into world boxes - and for stuffing a loaded region's
// chunk columns into a tree - lives here instead of being copy-pasted around.
#include "bvh.h"
aabb bvh_chunk_bounds(int cx, int cz);
aabb bvh_block_bounds(int bx, int by, int bz);
int  bvh_build_chunk_region(bvh *b, int center_cx, int center_cz, int radius);
uint32_t bvh_pack_chunk_id(int cx, int cz);
void     bvh_unpack_chunk_id(uint32_t id, int *cx, int *cz);
bvh_prim bvh_prim_box(uint32_t id, uint32_t tag, vec3 center, vec3 half);
#endif
