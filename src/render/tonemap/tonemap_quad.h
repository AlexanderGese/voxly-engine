#ifndef RENDER_TONEMAP_QUAD_H
#define RENDER_TONEMAP_QUAD_H

#include "../gl.h"

// the obligatory fullscreen triangle. yes, every post module in this tree has
// its own copy. no, i'm not going to refactor a 30-line vao into a shared one
// just to save three vbos.

typedef struct {
    glid vao;
    glid vbo;
} tonemap_quad;

int  tonemap_quad_create(tonemap_quad *q);
void tonemap_quad_destroy(tonemap_quad *q);
void tonemap_quad_draw(const tonemap_quad *q);

#endif
