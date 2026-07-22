#include "vol_quad.h"

// one triangle that covers the whole viewport once clipped. cheaper than a
// quad (no diagonal seam, one fewer vertex, better cache behaviour on the
// rasterizer) and you stop caring once you've typed it the tenth time.
static const float fs_tri[] = { -1.0f, -1.0f,
                                 3.0f, -1.0f,
                                -1.0f,  3.0f };

int volumetric_quad_init(volumetric_quad *q) {
    glGenVertexArrays(1, &q->vao);
    glGenBuffers(1, &q->vbo);
    glBindVertexArray(q->vao);
    glBindBuffer(GL_ARRAY_BUFFER, q->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof fs_tri, fs_tri, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
    return (q->vao != 0);
}

void volumetric_quad_destroy(volumetric_quad *q) {
    if (q->vao) glDeleteVertexArrays(1, &q->vao);
    if (q->vbo) glDeleteBuffers(1, &q->vbo);
    q->vao = 0;
    q->vbo = 0;
}

void volumetric_quad_draw(const volumetric_quad *q) {
    glBindVertexArray(q->vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}
