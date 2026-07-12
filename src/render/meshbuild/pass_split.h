#ifndef RENDER_MESHBUILD_PASS_SPLIT_H
#define RENDER_MESHBUILD_PASS_SPLIT_H

// translucent blocks (water, glass, ice) have to be drawn after all the opaque
// geometry, with depth-write off and back-to-front-ish, or they punch holes in
// the world. so a chunk really needs two meshes, not one. this splits a block
// into "which pass does it belong to" and provides a two-bucket result the
// builder can fill.

#include "mb_types.h"

typedef enum {
    MB_PASS_OPAQUE = 0,
    MB_PASS_TRANSLUCENT,
    MB_PASS_COUNT
} mb_pass;

// which render pass a block's faces go in. air is opaque-by-default but never
// emits faces so it doesnt matter. glass/water/ice/leaves-with-alpha land in
// translucent.
mb_pass mb_pass_of(block_id id);

// a built chunk = one result per pass.
typedef struct {
    mb_result pass[MB_PASS_COUNT];
} mb_chunk_mesh;

void mb_chunk_mesh_init(mb_chunk_mesh *m);
void mb_chunk_mesh_free(mb_chunk_mesh *m);

// total verts across both passes, for stats / buffer sizing.
int  mb_chunk_mesh_verts(const mb_chunk_mesh *m);

#endif
