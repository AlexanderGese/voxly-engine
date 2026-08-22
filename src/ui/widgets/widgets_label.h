#ifndef UI_WIDGETS_LABEL_H
#define UI_WIDGETS_LABEL_H
// text widgets. these don't interact, they just emit a WG_CMD_TEXT into the
// draw list at the right spot for the given alignment. the real glyph shaping
// happens host-side when it drains the command buffer; here we only need a
// rough advance width to place aligned/right-justified text, so we carry a
// cheap monospace-ish metric. it's wrong by a pixel or two for proportional
// fonts but for laying out labels in a panel it's invisible.
#include "widgets_context.h"
#include "widgets_layout.h"
// nominal glyph cell at scale 1.0, in px. matches the 6x8 bitmap font the hud
// fell back to historically; the proportional font is close enough on average.
#define WG_GLYPH_W 6.0f
#define WG_GLYPH_H 8.0f
typedef enum {
    WG_TEXT_LEFT = 0,
    WG_TEXT_CENTER,
    WG_TEXT_RIGHT,
} wg_text_align;
// estimate the rendered width of `s` at `scale`. used internally for alignment
// and exposed because button/slider want to center their captions too.
float wg_text_width(const char *s, float scale);
// draw `s` aligned within rect `r` (vertically centered), tinted `color`.
void wg_label_in(wg_context *ctx, wg_rect r, const char *s,
                 wg_text_align align, wg_rgba color);
// convenience: consume a layout row and draw a left label into it.
void wg_label(wg_context *ctx, wg_layout *l, const char *s);
// a dim secondary label (hints, units, etc).
void wg_label_dim(wg_context *ctx, wg_layout *l, const char *s);
// "key: value" line — label left, value right-justified in the same row. the
// staple of any debug/settings panel.
void wg_label_kv(wg_context *ctx, wg_layout *l, const char *key, const char *value);
// a horizontal separator line spanning the content width.
void wg_separator(wg_context *ctx, wg_layout *l);
#endif
