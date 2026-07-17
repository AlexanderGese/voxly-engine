#include "ssr_fsquad.h"

#include <stddef.h>

// clip-space oversized triangle. covers [-1,1]^2 with a single primitive.
static const float fs_tri[] = { -1.0f, -1.0f,
                                 3.0f, -1.0f,
                                -1.0f,  3.0f };

int ssrx_fsquad_init(ssrx_fsquad *q) {
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

void ssrx_fsquad_destroy(ssrx_fsquad *q) {
    if (q->vao) glDeleteVertexArrays(1, &q->vao);
    if (q->vbo) glDeleteBuffers(1, &q->vbo);
    q->vao = 0;
    q->vbo = 0;
}

void ssrx_fsquad_draw(const ssrx_fsquad *q) {
    glBindVertexArray(q->vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}
