#include "widgets_checkbox.h"
#include "widgets_label.h"
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
wg_rect cap;
wg_rect box = box_geom(ctx, row, &cap);
wg_rgba bg = held ? ctx->style.widget_active
                      : (hovered ? ctx->style.widget_hover : ctx->style.widget_bg);
wg_draw_rect(&ctx->draw, box, bg);
wg_draw_border(&ctx->draw, box, ctx->style.widget_border, ctx->style.border_thick);
return changed;
