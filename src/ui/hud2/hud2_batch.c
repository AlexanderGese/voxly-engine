#include "hud2_batch.h"
#include <math.h>
#include <stddef.h>
static void push_vert(hud2_batch *b, float x, float y, hud2_color c) {
    if (b->count * HUD2_FLOATS_PER_VERT + HUD2_FLOATS_PER_VERT > HUD2_BATCH_MAX_VERTS)
        hud2_batch_flush(b);
    float *v = &b->verts[b->count * HUD2_FLOATS_PER_VERT];
    v[0] = x; v[1] = y;
    v[2] = c.r; v[3] = c.g; v[4] = c.b; v[5] = c.a;
    b->count++;
}

void hud2_batch_init(hud2_batch *b, glid prog) {
    if (b->ready) return;
b->prog  = prog;
b->count = 0;
b->draws = 0;
glGenVertexArrays(1, &b->vao);
glGenBuffers(1, &b->vbo);
glBindVertexArray(b->vao);
glBindBuffer(GL_ARRAY_BUFFER, b->vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(b->verts), NULL, GL_STREAM_DRAW);
GLsizei stride = HUD2_FLOATS_PER_VERT * sizeof(float);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride,
                          (void*)(2 * sizeof(float)));
glBindVertexArray(0);
b->ready = 1;
}

void hud2_batch_destroy(hud2_batch *b) {
    if (!b->ready) return;
    glDeleteBuffers(1, &b->vbo);
    glDeleteVertexArrays(1, &b->vao);
    b->ready = 0;
}

void hud2_batch_begin(hud2_batch *b, int sw, int sh) {
    b->sw = sw;
b->sh = sh;
b->count = 0;
b->draws = 0;
push_vert(b, x1, y1, c);
push_vert(b, x2, y2, c);
hud2_batch_quad(b, x,         y + h - t, w, t, c);
hud2_batch_quad(b, x,         y,         t, h, c);
hud2_batch_quad(b, x + w - t, y,         t, h, c);
glBindVertexArray(0);
}
