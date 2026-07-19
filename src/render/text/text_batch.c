#include "text_batch.h"
#include "../../config.h"
#include "../../util/darray.h"
#include "../../util/log.h"

#include <stddef.h>
#include <string.h>

int text_batch_init(text_batch *b, glid prog) {
    memset(b, 0, sizeof *b);
    b->prog = prog;

    glGenVertexArrays(1, &b->vao);
    glGenBuffers(1, &b->vbo);

    glBindVertexArray(b->vao);
    glBindBuffer(GL_ARRAY_BUFFER, b->vbo);
    // orphan a chunk up front so the first frame doesnt stall reallocating.
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(TEXT_BATCH_MAX_QUADS * 6 * sizeof(text_vertex)),
                 NULL, GL_DYNAMIC_DRAW);

    // attrib 0: pos (vec2), attrib 1: uv (vec2), attrib 2: color (rgba8 norm)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(text_vertex),
                          (void*)offsetof(text_vertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(text_vertex),
                          (void*)offsetof(text_vertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(text_vertex),
                          (void*)offsetof(text_vertex, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    b->screen_w = (float)WIN_WIDTH;   // sane default until set_screen
    b->screen_h = (float)WIN_HEIGHT;
    return 1;
}

void text_batch_destroy(text_batch *b) {
    if (b->vao) glDeleteVertexArrays(1, &b->vao);
    if (b->vbo) glDeleteBuffers(1, &b->vbo);
    darr_free(b->verts);
    memset(b, 0, sizeof *b);
}

void text_batch_reset(text_batch *b) {
    darr_clear(b->verts);
    b->quad_count = 0;
}

void text_batch_set_screen(text_batch *b, float w, float h) {
    b->screen_w = w > 0 ? w : 1.0f;
    b->screen_h = h > 0 ? h : 1.0f;
}

void text_batch_push_quad(text_batch *b,
                          float x0, float y0, float x1, float y1,
                          float u0, float v0, float u1, float v1,
                          text_rgba color) {
    if (b->quad_count >= TEXT_BATCH_MAX_QUADS) {
        // hit the cap mid-frame. rather than silently drop, flush would be ideal
        // but we dont have the tex here, so just bail — the caller over-asked.
        return;
    }
    // two triangles, 6 verts, ccw. top-left origin so y0 < y1.
    text_vertex tl = { x0, y0, u0, v0, color };
    text_vertex tr = { x1, y0, u1, v0, color };
    text_vertex br = { x1, y1, u1, v1, color };
    text_vertex bl = { x0, y1, u0, v1, color };

    darr_push(b->verts, tl);
    darr_push(b->verts, bl);
    darr_push(b->verts, br);
    darr_push(b->verts, tl);
    darr_push(b->verts, br);
    darr_push(b->verts, tr);
    b->quad_count++;
}

void text_batch_push_layout(text_batch *b, const text_font *font,
                            const text_layout *l, float ox, float oy,
                            text_rgba color) {
    for (int i = 0; i < l->count; i++) {
        const text_placement *pl = &l->items[i];
        const text_glyph *g = pl->g;
        if (g->w == 0 || g->h == 0) continue; // blanks have no quad

        float u0, v0, u1, v1;
        text_atlas_uv(&font->atlas, &g->rect, &u0, &v0, &u1, &v1);

        float x0 = ox + pl->x;
        float y0 = oy + pl->y;
        text_batch_push_quad(b, x0, y0, x0 + g->w, y0 + g->h,
                             u0, v0, u1, v1, color);
    }
}

void text_batch_push_layout_shadow(text_batch *b, const text_font *font,
                                   const text_layout *l, float ox, float oy,
                                   text_rgba color, text_rgba shadow,
                                   float dx, float dy) {
    // shadow underneath first so the main glyphs draw over it. same batch, so
    // it's still one draw call.
    text_batch_push_layout(b, font, l, ox + dx, oy + dy, shadow);
    text_batch_push_layout(b, font, l, ox, oy, color);
}

void text_batch_flush(text_batch *b, glid tex) {
    int vcount = (int)darr_len(b->verts);
    if (vcount == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, b->vbo);
    // re-orphan if we overflowed the preallocated size; otherwise sub-update.
    GLsizeiptr bytes = (GLsizeiptr)vcount * sizeof(text_vertex);
    GLsizeiptr cap   = (GLsizeiptr)(TEXT_BATCH_MAX_QUADS * 6 * sizeof(text_vertex));
    if (bytes > cap) {
        glBufferData(GL_ARRAY_BUFFER, bytes, b->verts, GL_DYNAMIC_DRAW);
    } else {
        glBufferData(GL_ARRAY_BUFFER, cap, NULL, GL_DYNAMIC_DRAW); // orphan
        glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, b->verts);
    }

    glUseProgram(b->prog);
    gl_set_uniform_float(b->prog, "u_screen_w", b->screen_w);
    gl_set_uniform_float(b->prog, "u_screen_h", b->screen_h);
    gl_set_uniform_int(b->prog, "u_atlas", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(b->vao);
    glDrawArrays(GL_TRIANGLES, 0, vcount);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    text_batch_reset(b);
}
