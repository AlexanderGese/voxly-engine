#include "menus_controls.h"
#include <stdio.h>
#include <string.h>
#include "../widgets/widgets_label.h"
// pull a control color for the current hot/active/focus state. focus (keyboard)
// reads brighter than plain hot so you can tell which one the arrows will hit.
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
}

int menus_ctl_toggle(wg_context *ctx, wg_layout *l, menus_nav *nav,
                     const char *label, int *value) {
    wg_rect r  = wg_layout_row(l, ctx, 0);
    wg_id   id = wg_gen_id(ctx, label);

    int focused = menus_nav_item(nav, id, MENUS_NO_ITEM);
    menus_nav_dir intent = menus_nav_take_intent(nav, id);

    int hovered = 0, held = 0;
    int clicked = wg_behavior(ctx, id, r, &hovered, &held);
    // left/right also flip a toggle — feels natural with arrow nav.
    if (intent == MENUS_NAV_ACTIVATE || intent == MENUS_NAV_INC ||
        intent == MENUS_NAV_DEC)
        clicked = 1;

    int changed = 0;
    if (clicked) { *value = !*value; changed = 1; }

    draw_surface(ctx, r, hovered, held, focused, 1);
    wg_label_in(ctx, r, label, WG_TEXT_LEFT, ctx->style.text);

    // value chip on the right edge.
    const char *vs = *value ? "on" : "off";
    wg_rgba vc = *value ? ctx->style.accent : ctx->style.text_dim;
    wg_rect chip = r;
    chip.x = r.x + r.w - 56.0f; chip.w = 48.0f;
    wg_label_in(ctx, chip, vs, WG_TEXT_RIGHT, vc);
    return changed;
}

int menus_ctl_slider(wg_context *ctx, wg_layout *l, menus_nav *nav,
                     const char *label, float *value, float lo, float hi,
                     float step) {
    wg_rect r  = wg_layout_row(l, ctx, 0);
wg_id   id = wg_gen_id(ctx, label);
int focused = menus_nav_item(nav, id, MENUS_NO_ITEM);
menus_nav_dir intent = menus_nav_take_intent(nav, id);
int hovered = 0, held = 0;
wg_behavior(ctx, id, r, &hovered, &held);
float span = hi - lo;
if (span <= 0.0f) span = 1.0f;
// guard a bad caller
float before = *value;
// the draggable track is the right ~55% of the row; the left holds the label.
float track_x0 = r.x + r.w * 0.42f;
float track_x1 = r.x + r.w - ctx->style.pad;
float track_w  = track_x1 - track_x0;
if (track_w < 8.0f) track_w = 8.0f;
if (held) {
        // map cursor x onto the track, anchored so the handle doesn't jump.
        float t = (ctx->input.mouse_x - track_x0) / track_w;
        *value = lo + clampf(t, 0.0f, 1.0f) * span;
    }
    // keyboard nudge.
    if (intent == MENUS_NAV_INC) *value += step;
if (intent == MENUS_NAV_DEC) *value -= step;
*value = clampf(*value, lo, hi);
// background row + label.
draw_surface(ctx, r, hovered, held, focused, 1);
wg_label_in(ctx, r, label, WG_TEXT_LEFT, ctx->style.text);
// track line, vertically centered in the row.
float ty = r.y + r.h * 0.5f;
wg_draw_line(&ctx->draw, track_x0, ty, track_x1, ty,
                 ctx->style.widget_border, 2.0f);
// filled portion up to the handle.
float frac = (*value - lo) / span;
float hx = track_x0 + clampf(frac, 0.0f, 1.0f) * track_w;
wg_draw_line(&ctx->draw, track_x0, ty, hx, ty, ctx->style.accent, 2.0f);
// the handle: a small box centered on hx.
float g = ctx->style.slider_grab;
wg_rect handle = wg_rect_make(hx - g, ty - g, g * 2.0f, g * 2.0f);
wg_rgba hc = (held || focused) ? ctx->style.accent : ctx->style.text_dim;
wg_draw_rect(&ctx->draw, handle, hc);
return *value != before;
}

int menus_ctl_spinner(wg_context *ctx, wg_layout *l, menus_nav *nav,
                      const char *label, int *value, int lo, int hi) {
    wg_rect r  = wg_layout_row(l, ctx, 0);
    wg_id   id = wg_gen_id(ctx, label);

    int focused = menus_nav_item(nav, id, MENUS_NO_ITEM);
    menus_nav_dir intent = menus_nav_take_intent(nav, id);

    int before = *value;
    int stepn  = ctx->input.shift ? 5 : 1;

    // two chevron hit-boxes on the right. give them salted ids so they don't
    // collide with the row id or each other.
    float bw = 22.0f;
    wg_rect right = wg_rect_make(r.x + r.w - bw - ctx->style.pad,
                                 r.y, bw, r.h);
    wg_rect left  = wg_rect_make(right.x - bw - 4.0f, r.y, bw, r.h);

    wg_id ld = wg_gen_id_n(ctx, label, 1);
    wg_id rd = wg_gen_id_n(ctx, label, 2);
    int lh = 0, rh = 0;
    if (wg_behavior(ctx, ld, left,  &lh, NULL)) *value -= stepn;
    if (wg_behavior(ctx, rd, right, &rh, NULL)) *value += stepn;

    if (intent == MENUS_NAV_DEC) *value -= stepn;
    if (intent == MENUS_NAV_INC) *value += stepn;
    *value = clampi(*value, lo, hi);

    draw_surface(ctx, r, 0, 0, focused, 1);
    wg_label_in(ctx, r, label, WG_TEXT_LEFT, ctx->style.text);

    // value sits between the chevrons.
    char buf[16];
    snprintf(buf, sizeof buf, "%d", *value);
    wg_rect mid = wg_rect_make(r.x, r.y, left.x - r.x - 4.0f, r.h);
    wg_label_in(ctx, mid, buf, WG_TEXT_RIGHT, ctx->style.text);

    // chevrons.
    wg_draw_rect(&ctx->draw, left,  lh ? ctx->style.widget_hover : ctx->style.widget_bg);
    wg_draw_rect(&ctx->draw, right, rh ? ctx->style.widget_hover : ctx->style.widget_bg);
    wg_label_in(ctx, left,  "<", WG_TEXT_CENTER, ctx->style.text);
    wg_label_in(ctx, right, ">", WG_TEXT_CENTER, ctx->style.text);

    return *value != before;
}

void menus_ctl_header(wg_context *ctx, wg_layout *l, const char *text) {
    wg_rect r = wg_layout_row(l, ctx, ctx->style.row_height * 0.9f);
// a faint underline plus the text, no surface — reads as a section break.
wg_label_in(ctx, r, text, WG_TEXT_LEFT, ctx->style.accent_dim);
float by = r.y + r.h - 2.0f;
wg_draw_line(&ctx->draw, r.x, by, r.x + r.w, by,
                 ctx->style.panel_border, 1.0f);
}
