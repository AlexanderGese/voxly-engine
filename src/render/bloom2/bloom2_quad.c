#include "bloom2_quad.h"

#include <stddef.h>

// clip-space fullscreen triangle. the vertex shader reconstructs uv from
// gl_Position so we only need positions here. third vertex pokes past the
// screen on purpose — the rasterizer clips it.
static const float fs_tri[] = {
    -1.0f, -1.0f,
     3.0f, -1.0f,
    -1.0f,  3.0f,
};

int bloom2_quad_create(bloom2_quad *q) {
    glGenVertexArrays(1, &q->vao);
    glGenBuffers(1, &q->vbo);

    glBindVertexArray(q->vao);
    glBindBuffer(GL_ARRAY_BUFFER, q->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof fs_tri, fs_tri, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
    return 1;
}

void bloom2_quad_destroy(bloom2_quad *q) {
    if (q->vao) glDeleteVertexArrays(1, &q->vao);
    if (q->vbo) glDeleteBuffers(1, &q->vbo);
    q->vao = 0;
    q->vbo = 0;
}

void bloom2_quad_draw(const bloom2_quad *q) {
    glBindVertexArray(q->vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
