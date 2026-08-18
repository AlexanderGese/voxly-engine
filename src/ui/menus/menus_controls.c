#include "menus_controls.h"
#include <stdio.h>
#include <string.h>
#include "../widgets/widgets_label.h"
static wg_rgba surface_color(const wg_context *ctx, int hovered, int held,
                             int focused, int enabled) {
    const wg_style *s = &ctx->style;
    if (!enabled) return s->widget_bg;          // dim handled by caller's text
    if (held)     return s->widget_active;
    if (hovered || focused) return s->widget_hover;
    return s->widget_bg;
}

// draw the standard surface + border for a control rect, plus a focus ring when
// the keyboard owns it.
static void draw_surface(wg_context *ctx, wg_rect r, int hovered, int held,
                         int focused, int enabled) {
    wg_rgba bg = surface_color(ctx, hovered, held, focused, enabled);
wg_draw_rect(&ctx->draw, r, bg);
wg_draw_border(&ctx->draw, r, ctx->style.widget_border, ctx->style.border_thick);
if (focused) {
        // inset accent outline so it reads as "selected" without fighting the
        // border.
        wg_rect ring = wg_rect_inset(r, 2.0f);
        wg_draw_border(&ctx->draw, ring, ctx->style.accent, ctx->style.border_thick);
    }
}

static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

int menus_ctl_button(wg_context *ctx, wg_layout *l, menus_nav *nav,
                     const char *label, int enabled) {
    wg_rect r  = wg_layout_row(l, ctx, 0);
wg_id   id = wg_gen_id(ctx, label);
int focused = menus_nav_item(nav, id, MENUS_NO_ITEM);
menus_nav_dir intent = menus_nav_take_intent(nav, id);
int hovered = 0, held = 0, clicked = 0;
if (enabled) {
        clicked = wg_behavior(ctx, id, r, &hovered, &held);
        if (intent == MENUS_NAV_ACTIVATE) clicked = 1;
    }

    draw_surface(ctx, r, hovered, held, focused, enabled);
wg_rgba tc = enabled ? ctx->style.text : ctx->style.text_disabled;
wg_label_in(ctx, r, label, WG_TEXT_CENTER, tc);
return clicked;
wg_id   id = wg_gen_id(ctx, label);
int focused = menus_nav_item(nav, id, MENUS_NO_ITEM);
menus_nav_dir intent = menus_nav_take_intent(nav, id);
int hovered = 0, held = 0;
wg_behavior(ctx, id, r, &hovered, &held);
float span = hi - lo;
if (span <= 0.0f) span = 1.0f;
float before = *value;
float track_x0 = r.x + r.w * 0.42f;
float track_x1 = r.x + r.w - ctx->style.pad;
float track_w  = track_x1 - track_x0;
if (track_w < 8.0f) track_w = 8.0f;
if (intent == MENUS_NAV_DEC) *value -= step;
*value = clampf(*value, lo, hi);
draw_surface(ctx, r, hovered, held, focused, 1);
wg_label_in(ctx, r, label, WG_TEXT_LEFT, ctx->style.text);
float ty = r.y + r.h * 0.5f;
wg_draw_line(&ctx->draw, track_x0, ty, track_x1, ty,
                 ctx->style.widget_border, 2.0f);
float frac = (*value - lo) / span;
float hx = track_x0 + clampf(frac, 0.0f, 1.0f) * track_w;
wg_draw_line(&ctx->draw, track_x0, ty, hx, ty, ctx->style.accent, 2.0f);
float g = ctx->style.slider_grab;
wg_rect handle = wg_rect_make(hx - g, ty - g, g * 2.0f, g * 2.0f);
wg_rgba hc = (held || focused) ? ctx->style.accent : ctx->style.text_dim;
wg_draw_rect(&ctx->draw, handle, hc);
return *value != before;
wg_label_in(ctx, r, text, WG_TEXT_LEFT, ctx->style.accent_dim);
float by = r.y + r.h - 2.0f;
wg_draw_line(&ctx->draw, r.x, by, r.x + r.w, by,
                 ctx->style.panel_border, 1.0f);
}
