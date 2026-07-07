#ifndef RENDER_FXAA_QUAD_H
#define RENDER_FXAA_QUAD_H

#include "../gl.h"

// fullscreen triangle for the post pass. one oversized tri instead of a quad
// so there's no diagonal seam where two triangles meet and so we issue a
// single draw with three verts. same trick the rest of the post stack uses.

typedef struct {
    glid vao;
    glid vbo;
} fxaa_quad;

int  fxaa_quad_create(fxaa_quad *q);
void fxaa_quad_destroy(fxaa_quad *q);
void fxaa_quad_draw(const fxaa_quad *q);

#endif
