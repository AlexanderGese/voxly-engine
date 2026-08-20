#include "widgets_combo.h"
#include "widgets_label.h"

#define WG_COMBO_ITEM_H  18.0f

int wg_combo_box(wg_context *ctx, wg_layout *l, const char *label, wg_combo *c,
                 const char *const *items, int count, int *sel) {
    const wg_style *s = &ctx->style;
    wg_rect row = wg_layout_row(l, ctx, 0);

    // optional left caption taking a third, like the sliders do.
    wg_rect box = row;
    if (label && label[0]) {
        wg_rect lr = wg_rect_make(row.x, row.y, row.w * 0.34f, row.h);
        box = wg_rect_make(row.x + row.w * 0.36f, row.y, row.w * 0.64f, row.h);
        wg_label_in(ctx, lr, label, WG_TEXT_LEFT, s->text_dim);
    }

    wg_id id = wg_gen_id(ctx, label ? label : "##combo");
    int hovered = 0, held = 0;
    if (wg_behavior(ctx, id, box, &hovered, &held))
        c->open = !c->open;

    // closed box: current selection + a little down-chevron on the right.
    wg_rgba bg = held ? s->widget_active : (hovered ? s->widget_hover : s->widget_bg);
    wg_draw_rect(&ctx->draw, box, bg);
    wg_draw_border(&ctx->draw, box, s->widget_border, s->border_thick);

    const char *cur = (sel && *sel >= 0 && *sel < count) ? items[*sel] : "";
    wg_rect cap = wg_rect_make(box.x + 4.0f, box.y, box.w - 18.0f, box.h);
    wg_label_in(ctx, cap, cur, WG_TEXT_LEFT, s->text);

    wg_rect chev = wg_rect_make(box.x + box.w - WG_COMBO_ITEM_H, box.y,
                                WG_COMBO_ITEM_H, box.h);
    wg_label_in(ctx, chev, c->open ? "^" : "v", WG_TEXT_CENTER, s->text_dim);

    if (!c->open) return 0;

    // dropped list, growing downward from the box. each item is its own hit-
    // testable strip. we reserve the vertical space in the layout too so a combo
    // opened mid-panel pushes following widgets down instead of drawing over them.
    int changed = 0;
    c->hover_item = -1;
    float listy = box.y + box.h;
    float listh = WG_COMBO_ITEM_H * (float)count;
    wg_rect list = wg_rect_make(box.x, listy, box.w, listh);

    wg_draw_rect(&ctx->draw, list, s->panel_bg);

    for (int i = 0; i < count; i++) {
        wg_rect item = wg_rect_make(box.x, listy + WG_COMBO_ITEM_H * (float)i,
                                    box.w, WG_COMBO_ITEM_H);
        wg_id iid = wg_gen_id_n(ctx, label ? label : "##combo", i + 1);
        int ih = 0, ihd = 0;
        int click = wg_behavior(ctx, iid, item, &ih, &ihd);
        if (ih) c->hover_item = i;

        if (ih) wg_draw_rect(&ctx->draw, item, s->widget_hover);
        else if (sel && *sel == i)
            wg_draw_rect(&ctx->draw, item, s->accent_dim);

        wg_rect ic = wg_rect_make(item.x + 4.0f, item.y, item.w - 6.0f, item.h);
        wg_label_in(ctx, ic, items[i], WG_TEXT_LEFT, s->text);

        if (click) {
            if (sel && *sel != i) { *sel = i; changed = 1; }
            c->open = 0;
        }
    }
    wg_draw_border(&ctx->draw, list, s->panel_border, s->border_thick);

    // click anywhere off the list (and off the box) closes it without picking.
    if (wg_input_mouse_pressed(&ctx->input, WG_MOUSE_LEFT) &&
        c->hover_item < 0 &&
        !wg_input_over(&ctx->input, box)) {
        c->open = 0;
    }

    // eat the layout space the open list occupies.
    wg_layout_gap(l, listh);
    return changed;
}
