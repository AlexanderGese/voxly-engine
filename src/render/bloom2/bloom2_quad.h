#ifndef RENDER_BLOOM2_QUAD_H
#define RENDER_BLOOM2_QUAD_H

#include "../gl.h"

// a fullscreen triangle. one big tri that covers the screen is cheaper than
// two and avoids the diagonal seam quads sometimes show. every post pass in
// the chain draws this same vao.

typedef struct {
    glid vao;
    glid vbo;
} bloom2_quad;

int  bloom2_quad_create(bloom2_quad *q);
void bloom2_quad_destroy(bloom2_quad *q);

// bind + draw. caller is expected to have a program bound already.
void bloom2_quad_draw(const bloom2_quad *q);

#endif
