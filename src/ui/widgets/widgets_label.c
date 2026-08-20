#include "widgets_label.h"

#include <string.h>

float wg_text_width(const char *s, float scale) {
    if (!s) return 0.0f;
    // monospace-ish estimate: glyph cell width times count. tabs count as 4,
    // newlines reset — but labels are single-line so we just stop at one.
    float w = 0.0f;
    for (const char *p = s; *p && *p != '\n'; p++) {
        if (*p == '\t') w += WG_GLYPH_W * 4.0f;
        else            w += WG_GLYPH_W;
    }
    return w * scale;
}

void wg_label_in(wg_context *ctx, wg_rect r, const char *s,
                 wg_text_align align, wg_rgba color) {
    if (!s || !s[0]) return;
    float sc = ctx->style.font_scale;
    float tw = wg_text_width(s, sc);
    float th = WG_GLYPH_H * sc;

    float tx;
    switch (align) {
    case WG_TEXT_CENTER: tx = r.x + (r.w - tw) * 0.5f; break;
    case WG_TEXT_RIGHT:  tx = r.x + r.w - tw;          break;
    case WG_TEXT_LEFT:
    default:             tx = r.x;                     break;
    }
    // vertically center in the row. round to whole px so the bitmap font lands
    // on a texel boundary and doesn't shimmer.
    float ty = r.y + (r.h - th) * 0.5f;
    tx = (float)(int)(tx + 0.5f);
    ty = (float)(int)(ty + 0.5f);

    wg_draw_text(&ctx->draw, tx, ty, s, sc, color);
}

void wg_label(wg_context *ctx, wg_layout *l, const char *s) {
    wg_rect r = wg_layout_row(l, ctx, WG_GLYPH_H * ctx->style.font_scale + 4.0f);
    wg_label_in(ctx, r, s, WG_TEXT_LEFT, ctx->style.text);
}

void wg_label_dim(wg_context *ctx, wg_layout *l, const char *s) {
    wg_rect r = wg_layout_row(l, ctx, WG_GLYPH_H * ctx->style.font_scale + 4.0f);
    wg_label_in(ctx, r, s, WG_TEXT_LEFT, ctx->style.text_dim);
}

void wg_label_kv(wg_context *ctx, wg_layout *l, const char *key, const char *value) {
    wg_rect r = wg_layout_row(l, ctx, WG_GLYPH_H * ctx->style.font_scale + 4.0f);
    // key on the left, value flush right. they share the row so a long key can
    // run under the value — callers keep keys short, this is a debug panel staple.
    wg_label_in(ctx, r, key,   WG_TEXT_LEFT,  ctx->style.text_dim);
    wg_label_in(ctx, r, value, WG_TEXT_RIGHT, ctx->style.text);
}

void wg_separator(wg_context *ctx, wg_layout *l) {
    // a thin row with a single line through its middle. we eat half the normal
    // spacing on each side so separators don't feel cramped against widgets.
    float h = 7.0f;
    wg_rect r = wg_layout_row(l, ctx, h);
    float y = (float)(int)(r.y + h * 0.5f) + 0.5f;
    wg_draw_line(&ctx->draw, r.x, y, r.x + r.w, y, ctx->style.widget_border, 1.0f);
}
