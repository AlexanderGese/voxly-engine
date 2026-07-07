#include "fxaa_quad.h"

#include <stddef.h>

// clip-space fullscreen triangle. the vert shader derives uv from the clip
// position so we only ship positions. third vertex overshoots the screen;
// the rasterizer clips it back to the visible quad.
static const float fs_tri[] = {
    -1.0f, -1.0f,
     3.0f, -1.0f,
    -1.0f,  3.0f,
};

int fxaa_quad_create(fxaa_quad *q) {
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
    return q->vao != 0;
}

void fxaa_quad_destroy(fxaa_quad *q) {
    if (q->vao) glDeleteVertexArrays(1, &q->vao);
    if (q->vbo) glDeleteBuffers(1, &q->vbo);
    q->vao = 0;
    q->vbo = 0;
}

void fxaa_quad_draw(const fxaa_quad *q) {
    glBindVertexArray(q->vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}
