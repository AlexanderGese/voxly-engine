#include "hud2_batch.h"
#include <math.h>
#include <stddef.h>

// all geometry ends up as triangles. lines/outlines are expanded into thin
// quads so we never have to touch glLineWidth (which is deprecated/unreliable
// on core profile anyway, learned that the hard way).

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
}

void hud2_batch_quad(hud2_batch *b, float x, float y, float w, float h,
                     hud2_color c) {
    // two tris, ccw-ish, but we draw with culling off so winding is moot.
    push_vert(b, x,     y,     c);
    push_vert(b, x + w, y,     c);
    push_vert(b, x + w, y + h, c);
    push_vert(b, x,     y,     c);
    push_vert(b, x + w, y + h, c);
    push_vert(b, x,     y + h, c);
}

void hud2_batch_tri(hud2_batch *b,
                    float x0, float y0, float x1, float y1,
                    float x2, float y2, hud2_color c) {
    push_vert(b, x0, y0, c);
    push_vert(b, x1, y1, c);
    push_vert(b, x2, y2, c);
}

void hud2_batch_line(hud2_batch *b, float x0, float y0, float x1, float y1,
                     float thick, hud2_color c) {
    // expand into a quad perpendicular to the segment.
    float dx = x1 - x0, dy = y1 - y0;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1e-4f) return;
    float nx = -dy / len * (thick * 0.5f);
    float ny =  dx / len * (thick * 0.5f);
    push_vert(b, x0 + nx, y0 + ny, c);
    push_vert(b, x1 + nx, y1 + ny, c);
    push_vert(b, x1 - nx, y1 - ny, c);
    push_vert(b, x0 + nx, y0 + ny, c);
    push_vert(b, x1 - nx, y1 - ny, c);
    push_vert(b, x0 - nx, y0 - ny, c);
}

void hud2_batch_rect_outline(hud2_batch *b, float x, float y, float w, float h,
                             float t, hud2_color c) {
    // four border quads. corners overlap by t which is fine, alpha aside.
    hud2_batch_quad(b, x,         y,         w, t, c);          // top
    hud2_batch_quad(b, x,         y + h - t, w, t, c);          // bottom
    hud2_batch_quad(b, x,         y,         t, h, c);          // left
    hud2_batch_quad(b, x + w - t, y,         t, h, c);          // right
}

void hud2_batch_flush(hud2_batch *b) {
    if (b->count == 0) return;
    glBindVertexArray(b->vao);
    glBindBuffer(GL_ARRAY_BUFFER, b->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    b->count * HUD2_FLOATS_PER_VERT * sizeof(float), b->verts);
    glDrawArrays(GL_TRIANGLES, 0, b->count);
    b->count = 0;
    b->draws++;
}

void hud2_batch_end(hud2_batch *b) {
    hud2_batch_flush(b);
    glBindVertexArray(0);
}
