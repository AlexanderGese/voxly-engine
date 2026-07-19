#include "text_panel.h"
#include "../../util/log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// sample the dead-center of the baked white block so a solid fill is solid no
// matter what the sampler does at the edges.
static void white_uv(const text_font *f, float *u, float *v) {
    if (f->white.w == 0) { *u = 0; *v = 0; return; }
    float cx = f->white.x + f->white.w * 0.5f;
    float cy = f->white.y + f->white.h * 0.5f;
    *u = cx / (float)f->atlas.w;
    *v = cy / (float)f->atlas.h;
}

static void fill_rect(text_ctx *c, float x0, float y0, float x1, float y1,
                      text_rgba color) {
    float u, v;
    white_uv(&c->font, &u, &v);
    // degenerate uv (all corners the same texel) => flat color, no bleeding.
    text_batch_push_quad(&c->batch, x0, y0, x1, y1, u, v, u, v, color);
}

void text_panel_begin(text_panel *p, text_ctx *c, float x, float y) {
    memset(p, 0, sizeof *p);
    p->ctx      = c;
    p->x        = x;
    p->y        = y;
    p->start_x  = x;
    p->start_y  = y;
    p->row_step = text_ctx_line_height(c);
    if (p->row_step <= 0) p->row_step = 12.0f;
}

float text_panel_row_color(text_panel *p, const char *s, text_rgba color) {
    text_layout_opts o;
    memset(&o, 0, sizeof o);
    o.line_spacing = 1.0f;

    float w = 0, h = 0;
    text_ctx_draw_ex(p->ctx, s, p->x, p->y, &o, color, p->ctx->shadow, &w, &h);

    if (w > p->widest) p->widest = w;
    p->y += (h > 0 ? h : p->row_step);
    p->rows++;
    return w;
}

float text_panel_row(text_panel *p, const char *s) {
    return text_panel_row_color(p, s, p->ctx->default_color);
}

float text_panel_rowf(text_panel *p, const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    if (n < 0) return 0;
    return text_panel_row(p, buf);
}

void text_panel_gap(text_panel *p) {
    p->y += p->row_step * 0.5f;
}

float text_panel_height(const text_panel *p) {
    // how far the cursor has walked down from where it started
    return p->y - p->start_y;
}

void text_panel_tooltip(text_ctx *c, const char *s, float cx, float top,
                        text_rgba bg, text_rgba fg) {
    const float pad = 6.0f;

    float tw = 0, th = 0;
    text_ctx_measure(c, s, &tw, &th);

    float box_w = tw + pad * 2.0f;
    float box_h = th + pad * 2.0f;
    float x0 = cx - box_w * 0.5f;
    float y0 = top;

    // background first (same batch, draws under the glyphs we queue next).
    fill_rect(c, x0, y0, x0 + box_w, y0 + box_h, bg);

    // a 1px lighter top edge so it reads as a raised box. cheap bevel.
    text_rgba edge = text_rgba_make(255, 255, 255, 40);
    fill_rect(c, x0, y0, x0 + box_w, y0 + 1.0f, edge);

    // centered text inside the padding
    text_layout_opts o;
    memset(&o, 0, sizeof o);
    o.line_spacing = 1.0f;
    o.align        = TEXT_ALIGN_CENTER;
    o.max_width    = tw;
    text_ctx_draw_ex(c, s, x0 + pad, y0 + pad, &o, fg, 0, NULL, NULL);
}
