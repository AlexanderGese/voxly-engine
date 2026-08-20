#ifndef UI_SETTINGS_PANEL_H
#define UI_SETTINGS_PANEL_H

// small shared chrome helpers for the settings menu. nothing stateful — just the
// geometry bits that several files in here wanted and the widget layer didn't
// expose in quite the shape we needed (centered text without a layout, a one-off
// spacer that mutates a remaining rect, a default panel rect).

#include "../widgets/widgets_context.h"

// estimated rendered width of `s` at `scale`, using the same ~6px advance the
// text batch assumes for the bundled font. good enough for centering chrome.
float settings_text_width(const char *s, float scale);

// draw `s` centered (both axes) inside `r`. returns nothing; clips to nothing.
void  settings_text_centered(wg_context *ctx, wg_rect r, const char *s,
                             float scale, wg_rgba color);

// draw `s` left-aligned, vertically centered, padded `pad` from the left edge.
void  settings_text_left(wg_context *ctx, wg_rect r, const char *s,
                         float scale, float pad, wg_rgba color);

// eat `px` of height off the top of *r (mutates it). for inter-section gaps.
void  settings_panel_spacer(wg_rect *r, float px);

// a sensible default panel rect: centered in the screen, clamped to a max size
// so it doesn't sprawl on ultrawide. the host can ignore this and pass its own.
wg_rect settings_panel_default_rect(float screen_w, float screen_h);

#endif
