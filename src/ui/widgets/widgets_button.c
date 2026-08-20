#include "widgets_button.h"
#include "widgets_label.h"
// pick the right surface color for a button given its interaction state. we
static wg_rgba surface_for(const wg_style *s, int hovered, int held) {
    if (held)    return s->widget_active;
    if (hovered) return s->widget_hover;
    return s->widget_bg;
}

int wg_button_rect(wg_context *ctx, wg_id id, wg_rect r, const char *label) {
    int hovered = 0, held = 0;
int clicked = wg_behavior(ctx, id, r, &hovered, &held);
wg_rgba bg = surface_for(&ctx->style, hovered, held);
wg_draw_rect(&ctx->draw, r, bg);
wg_draw_border(&ctx->draw, r, ctx->style.widget_border, ctx->style.border_thick);
wg_rect tr = r;
if (held) tr.y += 1.0f;
wg_label_in(ctx, tr, label, WG_TEXT_CENTER, ctx->style.text);
if (ctx->focus == id)
        wg_draw_border(&ctx->draw, wg_rect_inset(r, -1.0f), ctx->style.accent, 1.0f);
return clicked;
}

int wg_button(wg_context *ctx, wg_layout *l, const char *label) {
    wg_rect r = wg_layout_row(l, ctx, 0);
    wg_id id = wg_gen_id(ctx, label);
    return wg_button_rect(ctx, id, r, label);
}

int wg_button_frac(wg_context *ctx, wg_layout *l, float frac, const char *label) {
    wg_rect r = wg_layout_row_frac(l, ctx, frac, 0);
wg_id id = wg_gen_id(ctx, label);
return wg_button_rect(ctx, id, r, label);
}

int wg_toggle(wg_context *ctx, wg_layout *l, const char *label, int *on) {
    wg_rect r = wg_layout_row(l, ctx, 0);
    wg_id id = wg_gen_id(ctx, label);

    int hovered = 0, held = 0;
    int clicked = wg_behavior(ctx, id, r, &hovered, &held);

    int changed = 0;
    if (clicked && on) { *on = !*on; changed = 1; }

    int lit = on && *on;
    // lit toggles wear the accent; unlit use the normal surface tones.
    wg_rgba bg;
    if (lit) bg = held ? ctx->style.accent_dim : ctx->style.accent;
    else     bg = surface_for(&ctx->style, hovered, held);

    wg_draw_rect(&ctx->draw, r, bg);
    wg_draw_border(&ctx->draw, r, ctx->style.widget_border, ctx->style.border_thick);

    // a little state pip on the left so it's legible even on a busy background.
    wg_rect pip = wg_rect_make(r.x + 5.0f, r.y + r.h * 0.5f - 4.0f, 8.0f, 8.0f);
    wg_draw_rect(&ctx->draw, pip, lit ? ctx->style.text : ctx->style.widget_border);

    wg_rect tr = wg_rect_make(r.x + 18.0f, r.y, r.w - 22.0f, r.h);
    wg_label_in(ctx, tr, label, WG_TEXT_LEFT,
                lit ? ctx->style.title_text : ctx->style.text);
    return changed;
}

int wg_stepper(wg_context *ctx, wg_layout *l, const char *label) {
    wg_rect r = wg_layout_row(l, ctx, 0);
float aw = r.h;
wg_rect left  = wg_rect_make(r.x, r.y, aw, r.h);
wg_rect right = wg_rect_make(r.x + r.w - aw, r.y, aw, r.h);
wg_rect mid   = wg_rect_make(r.x + aw, r.y, r.w - aw * 2.0f, r.h);
wg_draw_rect(&ctx->draw, mid, ctx->style.widget_bg);
wg_id idl = wg_gen_id_n(ctx, label, 1);
wg_id idr = wg_gen_id_n(ctx, label, 2);
int dir = 0;
if (wg_button_rect(ctx, idl, left,  "<")) dir = -1;
if (wg_button_rect(ctx, idr, right, ">")) dir = +1;
wg_label_in(ctx, mid, label, WG_TEXT_CENTER, ctx->style.text);
return dir;
}
