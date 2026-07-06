#ifndef RENDER_DOF_QUAD_H
#define RENDER_DOF_QUAD_H

#include "../gl.h"

// the obligatory fullscreen triangle. one oversized tri covering the screen,
// drawn by every pass in the dof chain. cheaper than a quad and no diagonal
// seam. the vertex shader reconstructs uv from gl_VertexID-ish positions, so
// the vbo only needs clip-space xy.

typedef struct {
    glid vao;
    glid vbo;
} dof_quad;

int  dof_quad_create(dof_quad *q);
void dof_quad_destroy(dof_quad *q);

// caller must have a program bound. binds the vao and issues the draw.
void dof_quad_draw(const dof_quad *q);

#endif
