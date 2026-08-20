#include "widgets_checkbox.h"
#include "widgets_label.h"

// shared geometry: the box is a square the height of the text run, left-aligned
// in the row with a little inset. returns the box rect, sets *cap to the
// remaining caption rect.
static wg_rect box_geom(const wg_context *ctx, wg_rect row, wg_rect *cap) {
    float s = WG_GLYPH_H * ctx->style.font_scale + 4.0f;
    if (s > row.h) s = row.h;
    float y = row.y + (row.h - s) * 0.5f;
    wg_rect box = wg_rect_make(row.x + 2.0f, y, s, s);
    if (cap)
        *cap = wg_rect_make(box.x + s + 6.0f, row.y,
                            row.w - (s + 10.0f), row.h);
    return box;
}

int wg_checkbox(wg_context *ctx, wg_layout *l, const char *label, int *checked) {
    wg_rect row = wg_layout_row(l, ctx, 0);
    wg_id id = wg_gen_id(ctx, label);

    int hovered = 0, held = 0;
    int clicked = wg_behavior(ctx, id, row, &hovered, &held);

    int changed = 0;
    if (clicked && checked) { *checked = !*checked; changed = 1; }
    int on = checked && *checked;

    wg_rect cap;
    wg_rect box = box_geom(ctx, row, &cap);

    wg_rgba bg = held ? ctx->style.widget_active
                      : (hovered ? ctx->style.widget_hover : ctx->style.widget_bg);
    wg_draw_rect(&ctx->draw, box, bg);
    wg_draw_border(&ctx->draw, box, ctx->style.widget_border, ctx->style.border_thick);

    // the "check" — a filled inner square in the accent. cheaper than rendering
    // a tick glyph and reads fine at this size.
    if (on) {
        wg_rect mark = wg_rect_inset(box, 3.0f);
        wg_draw_rect(&ctx->draw, mark, ctx->style.accent);
    }

    wg_label_in(ctx, cap, label, WG_TEXT_LEFT,
                hovered ? ctx->style.text : ctx->style.text_dim);
    return changed;
}

int wg_radio(wg_context *ctx, wg_layout *l, const char *label,
             int *selected, int index) {
    wg_rect row = wg_layout_row(l, ctx, 0);
    // salt the id with the index so identical option labels stay distinct.
    wg_id id = wg_gen_id_n(ctx, label, index);

    int hovered = 0, held = 0;
    int clicked = wg_behavior(ctx, id, row, &hovered, &held);

    int became = 0;
    if (clicked && selected && *selected != index) {
        *selected = index;
        became = 1;
    }
    int on = selected && *selected == index;

    wg_rect cap;
    wg_rect box = box_geom(ctx, row, &cap);

    // draw the radio as a box too (no circle primitive in the draw list), but
    // inset the "dot" more so it visually differs from a checkbox.
    wg_rgba bg = held ? ctx->style.widget_active
                      : (hovered ? ctx->style.widget_hover : ctx->style.widget_bg);
    wg_draw_rect(&ctx->draw, box, bg);
    wg_draw_border(&ctx->draw, box, ctx->style.widget_border, ctx->style.border_thick);
    if (on) {
        wg_rect dot = wg_rect_inset(box, 4.0f);
        wg_draw_rect(&ctx->draw, dot, ctx->style.accent);
    }

    wg_label_in(ctx, cap, label, WG_TEXT_LEFT,
                on ? ctx->style.text : ctx->style.text_dim);
    return became;
}
