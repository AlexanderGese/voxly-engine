#ifndef RENDER_DECALS_MESH_H
#define RENDER_DECALS_MESH_H

#include "decals_types.h"
#include "../gl.h"

// the gpu geometry for the pass: one unit cube (the projector volume) drawn
// instanced. the cube vbo never changes; the instance vbo is re-filled each
// frame with the visible decals' matrices + uv rects + params. matches the
// attribute locations baked into decals_glsl.

// per-instance gpu payload. tightly packed, memcpy'd into the instance vbo.
// matches the layout the vertex shader reads at locations 1..10.
typedef struct {
    float model[16];     // cube-local -> world
    float inv_model[16]; // world -> cube-local
    float uv_rect[4];    // uv0.xy, uv1.xy
    float params[4];     // alpha, angle_fade, roughness, flags-as-float
} decals_mesh_inst;

typedef struct {
    glid vao;
    glid cube_vbo;       // 36 verts, no index buffer (kept simple)
    glid inst_vbo;       // DECALS_BATCH_MAX instances, orphaned per upload
    int  inst_cap;       // == DECALS_BATCH_MAX
} decals_mesh;

// create the cube + instance buffers and wire the vertex array. returns 1 on
// success.
int  decals_mesh_create(decals_mesh *m);
void decals_mesh_destroy(decals_mesh *m);

// upload up to inst_cap instances. orphans the buffer (GL_STREAM_DRAW) so the
// driver doesnt stall on the previous frame's draw. count is clamped.
void decals_mesh_upload(decals_mesh *m, const decals_mesh_inst *insts, int count);

// draw `count` instances of the cube. assumes a program is already bound.
void decals_mesh_draw(const decals_mesh *m, int count);

#endif
