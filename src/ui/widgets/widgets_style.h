#ifndef UI_WIDGETS_STYLE_H
#define UI_WIDGETS_STYLE_H

// the look-and-feel knobs. one of these lives on the context. defaults are a
// flat dark theme that matches the rest of voxl's hud — slightly translucent
// panels so the world bleeds through. tweak per-screen if you want, it's just
// data.

#include "widgets_types.h"

typedef struct {
    // panel chrome
    wg_rgba panel_bg;
    wg_rgba panel_border;
    wg_rgba title_bg;
    wg_rgba title_text;

    // widget surfaces, three states each
    wg_rgba widget_bg;
    wg_rgba widget_hover;
    wg_rgba widget_active;
    wg_rgba widget_border;

    // accents (slider fill, checkbox tick, focus ring)
    wg_rgba accent;
    wg_rgba accent_dim;

    // text
    wg_rgba text;
    wg_rgba text_dim;
    wg_rgba text_disabled;

    // metrics, all in px
    float pad;            // inner padding inside panels
    float spacing;        // gap between stacked widgets
    float row_height;     // default widget row height
    float font_scale;     // glyph scale handed to draw_text
    float border_thick;
    float slider_grab;    // half-width of a slider handle
    float scrollbar_w;

    // how fast hover/press color tweens approach their target, per second.
    float anim_speed;
} wg_style;

// fill `s` with the default dark theme.
void wg_style_defaults(wg_style *s);

#endif
