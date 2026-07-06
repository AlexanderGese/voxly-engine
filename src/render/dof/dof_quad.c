#include "dof_quad.h"

#include <stddef.h>

// three verts that overshoot the [-1,1] clip box. the rasterizer clips the
// overshoot away and we're left with exactly the screen, no seam down the
// middle the way a two-tri quad gives.
static const float QUAD_VERTS[] = {
    -1.0f, -1.0f,
     3.0f, -1.0f,
    -1.0f,  3.0f,
};

int dof_quad_create(dof_quad *q) {
    q->vao = 0;
    q->vbo = 0;

    glGenVertexArrays(1, &q->vao);
    glGenBuffers(1, &q->vbo);

    glBindVertexArray(q->vao);
    glBindBuffer(GL_ARRAY_BUFFER, q->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTS), QUAD_VERTS, GL_STATIC_DRAW);

    // attrib 0: vec2 clip-space position.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return q->vao != 0;
}

void dof_quad_destroy(dof_quad *q) {
    if (q->vbo) glDeleteBuffers(1, &q->vbo);
    if (q->vao) glDeleteVertexArrays(1, &q->vao);
    q->vao = 0;
    q->vbo = 0;
}

void dof_quad_draw(const dof_quad *q) {
    glBindVertexArray(q->vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}
