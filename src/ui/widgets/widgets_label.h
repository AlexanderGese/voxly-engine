#ifndef UI_WIDGETS_LABEL_H
#define UI_WIDGETS_LABEL_H
// text widgets. these don't interact, they just emit a WG_CMD_TEXT into the
// draw list at the right spot for the given alignment. the real glyph shaping
// happens host-side when it drains the command buffer; here we only need a
// rough advance width to place aligned/right-justified text, so we carry a
#include "widgets_context.h"
#include "widgets_layout.h"
#define WG_GLYPH_W 6.0f
#define WG_GLYPH_H 8.0f
typedef enum {
    WG_TEXT_LEFT = 0,
    WG_TEXT_CENTER,
    WG_TEXT_RIGHT,
} wg_text_align;
float wg_text_width(const char *s, float scale);
void wg_label_in(wg_context *ctx, wg_rect r, const char *s,
                 wg_text_align align, wg_rgba color);
void wg_label(wg_context *ctx, wg_layout *l, const char *s);
void wg_label_dim(wg_context *ctx, wg_layout *l, const char *s);
void wg_label_kv(wg_context *ctx, wg_layout *l, const char *key, const char *value);
void wg_separator(wg_context *ctx, wg_layout *l);
#endif
