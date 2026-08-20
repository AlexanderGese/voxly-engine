#include "settings_panel.h"

#include <string.h>

// the bundled bitmap font advances ~6px per glyph at scale 1. the widget code
// bakes the same constant; if the font ever changes both want updating.
#define SETTINGS_GLYPH_ADV 6.0f
#define SETTINGS_GLYPH_H   8.0f

float settings_text_width(const char *s, float scale) {
    if (!s) return 0.0f;
    return (float)strlen(s) * SETTINGS_GLYPH_ADV * scale;
}

void settings_text_centered(wg_context *ctx, wg_rect r, const char *s,
                            float scale, wg_rgba color) {
    if (!s || !s[0]) return;
    float tw = settings_text_width(s, scale);
    float tx = r.x + (r.w - tw) * 0.5f;
    float ty = r.y + (r.h - SETTINGS_GLYPH_H * scale) * 0.5f;
    wg_draw_text(&ctx->draw, tx, ty, s, scale, color);
}

void settings_text_left(wg_context *ctx, wg_rect r, const char *s,
                        float scale, float pad, wg_rgba color) {
    if (!s || !s[0]) return;
    float ty = r.y + (r.h - SETTINGS_GLYPH_H * scale) * 0.5f;
    wg_draw_text(&ctx->draw, r.x + pad, ty, s, scale, color);
}

void settings_panel_spacer(wg_rect *r, float px) {
    if (px <= 0.0f) return;
    if (px > r->h) px = r->h;
    r->y += px;
    r->h -= px;
}

wg_rect settings_panel_default_rect(float screen_w, float screen_h) {
    // target ~70% of the screen but cap it so a 4k window doesn't get a
    // ridiculous panel; then center the result.
    float w = screen_w * 0.7f;
    float h = screen_h * 0.8f;
    if (w > 640.0f) w = 640.0f;
    if (h > 560.0f) h = 560.0f;
    // don't go absurdly small either — tiny windows still get a usable panel.
    if (w < 320.0f) w = screen_w < 320.0f ? screen_w : 320.0f;
    if (h < 280.0f) h = screen_h < 280.0f ? screen_h : 280.0f;
    float x = (screen_w - w) * 0.5f;
    float y = (screen_h - h) * 0.5f;
    return wg_rect_make(x, y, w, h);
}
