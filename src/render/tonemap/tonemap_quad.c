#include "tonemap_quad.h"

#include <stddef.h>

// clip-space tri that covers the screen; the third vertex hangs off-screen and
// the rasterizer clips it. vertex shader derives uv from gl_Position.
static const float fs_tri[] = {
    -1.0f, -1.0f,
     3.0f, -1.0f,
    -1.0f,  3.0f,
};

int tonemap_quad_create(tonemap_quad *q) {
    q->vao = 0;
    q->vbo = 0;
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

void tonemap_quad_destroy(tonemap_quad *q) {
    if (q->vao) glDeleteVertexArrays(1, &q->vao);
    if (q->vbo) glDeleteBuffers(1, &q->vbo);
    q->vao = 0;
    q->vbo = 0;
}

void tonemap_quad_draw(const tonemap_quad *q) {
    glBindVertexArray(q->vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
