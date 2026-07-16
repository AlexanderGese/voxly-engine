#include "ssao_fsquad.h"

// one oversized triangle in clip space. the passthrough vert maps these
// straight to gl_Position and derives uv = pos*0.5+0.5.
static const float fs_tri[] = { -1.0f, -1.0f,
                                 3.0f, -1.0f,
                                -1.0f,  3.0f };

int ssaox_fsquad_init(ssaox_fsquad *q) {
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

void ssaox_fsquad_destroy(ssaox_fsquad *q) {
    if (q->vao) glDeleteVertexArrays(1, &q->vao);
    if (q->vbo) glDeleteBuffers(1, &q->vbo);
    q->vao = 0;
    q->vbo = 0;
}

void ssaox_fsquad_draw(const ssaox_fsquad *q) {
    glBindVertexArray(q->vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}
