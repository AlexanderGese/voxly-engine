#include "widgets_style.h"

void wg_style_defaults(wg_style *s) {
    // flat dark, panels ~85% opaque so a bit of the world shows through.
    s->panel_bg      = wg_rgba_make(22, 24, 28, 218);
    s->panel_border  = wg_rgba_make(8,  9,  11, 255);
    s->title_bg      = wg_rgba_make(34, 38, 44, 235);
    s->title_text    = wg_rgba_make(232, 234, 238, 255);

    s->widget_bg     = wg_rgba_make(44, 48, 56, 255);
    s->widget_hover  = wg_rgba_make(58, 64, 74, 255);
    s->widget_active = wg_rgba_make(72, 80, 92, 255);
    s->widget_border = wg_rgba_make(14, 15, 18, 255);

    // a desaturated cyan-green, reads as "voxl" without being loud.
    s->accent        = wg_rgba_make(86, 178, 154, 255);
    s->accent_dim    = wg_rgba_make(54, 110, 96, 255);

    s->text          = wg_rgba_make(224, 226, 230, 255);
    s->text_dim      = wg_rgba_make(150, 154, 160, 255);
    s->text_disabled = wg_rgba_make(96, 99, 104, 255);

    s->pad         = 8.0f;
    s->spacing     = 6.0f;
    s->row_height  = 22.0f;
    s->font_scale  = 1.0f;
    s->border_thick = 1.0f;
    s->slider_grab = 5.0f;
    s->scrollbar_w = 8.0f;

    s->anim_speed  = 14.0f;
}
