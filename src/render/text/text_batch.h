#ifndef RENDER_TEXT_BATCH_H
#define RENDER_TEXT_BATCH_H

// a batched quad buffer for text. you push quads (or whole laid-out blocks)
// across a frame, then flush() once to draw the lot in a single draw call.
// vertices are CPU-accumulated in a darray and streamed to a persistent vbo.
// one batch is tied to one atlas texture — mixing fonts means mixing batches,
// but we only have one font so nobody cares.

#include "text_common.h"
#include "text_layout.h"
#include "text_atlas.h"

typedef struct {
    glid vao, vbo;
    glid prog;          // text shader, set by the ctx that owns the batch

    text_vertex *verts; // darray of accumulated verts (6 per quad)
    int   quad_count;   // quads pushed since last flush

    // screen size for the ortho transform, fed to the shader each flush.
    float screen_w, screen_h;
} text_batch;

int  text_batch_init(text_batch *b, glid prog);
void text_batch_destroy(text_batch *b);

// drop all accumulated geometry without drawing.
void text_batch_reset(text_batch *b);

void text_batch_set_screen(text_batch *b, float w, float h);

// push a single axis-aligned textured quad (already in screen px / atlas uv).
void text_batch_push_quad(text_batch *b,
                          float x0, float y0, float x1, float y1,
                          float u0, float v0, float u1, float v1,
                          text_rgba color);

// push every glyph of a laid-out block, translated by (ox, oy) and tinted.
// `font` supplies the atlas to compute uvs from.
void text_batch_push_layout(text_batch *b, const text_font *font,
                            const text_layout *l, float ox, float oy,
                            text_rgba color);

// same but draws a 1px (scaled) drop shadow underneath first. cheap legibility.
void text_batch_push_layout_shadow(text_batch *b, const text_font *font,
                                   const text_layout *l, float ox, float oy,
                                   text_rgba color, text_rgba shadow,
                                   float dx, float dy);

// upload + draw everything, then reset. binds `tex` as the glyph atlas.
void text_batch_flush(text_batch *b, glid tex);

#endif
