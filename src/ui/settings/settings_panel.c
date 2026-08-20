#include "settings_panel.h"
#include <string.h>
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
if (px > r->h) px = r->h;
r->y += px;
r->h -= px;
