#include "widgets_slider.h"
#include "widgets_label.h"

#include <stdio.h>
#include <math.h>

static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

int wg_slider_rect(wg_context *ctx, wg_id id, wg_rect r,
                   float *v, float vmin, float vmax, const char *fmt) {
    if (!v) return 0;
    if (vmax <= vmin) vmax = vmin + 1e-4f;   // avoid div-by-zero, degenerate range

    int hovered = 0, held = 0;
    wg_behavior(ctx, id, r, &hovered, &held);

    float old = *v;

    // the grab travels inside the track inset by its half-width so the handle
    // never hangs off the ends.
    float grab = ctx->style.slider_grab;
    float x0 = r.x + grab;
    float x1 = r.x + r.w - grab;
    float span = x1 - x0;
    if (span < 1.0f) span = 1.0f;

    // while held, drive the value straight from the cursor (absolute mapping).
    if (held) {
        float mx = clampf(ctx->input.mouse_x, x0, x1);
        float t = (mx - x0) / span;
        *v = vmin + t * (vmax - vmin);
    }
    // scroll-wheel fine adjust when hovered: one tick = 1/20th of the range.
    else if (hovered && ctx->input.scroll != 0.0f) {
        *v += ctx->input.scroll * (vmax - vmin) / 20.0f;
    }
    *v = clampf(*v, vmin, vmax);

    float t = (*v - vmin) / (vmax - vmin);

    // track
    wg_draw_rect(&ctx->draw, r, ctx->style.widget_bg);
    // filled portion up to the handle, in the accent color.
    wg_rect fill = wg_rect_make(r.x, r.y, (x0 - r.x) + t * span, r.h);
    wg_draw_rect(&ctx->draw, fill, held ? ctx->style.accent : ctx->style.accent_dim);
    wg_draw_border(&ctx->draw, r, ctx->style.widget_border, ctx->style.border_thick);

    // the handle
    float hx = x0 + t * span;
    wg_rect handle = wg_rect_make(hx - grab, r.y + 1.0f, grab * 2.0f, r.h - 2.0f);
    wg_rgba hc = held ? ctx->style.title_text : (hovered ? ctx->style.widget_hover
                                                         : ctx->style.widget_active);
    wg_draw_rect(&ctx->draw, handle, hc);

    // value readout centered over the track.
    if (fmt) {
        char buf[32];
        snprintf(buf, sizeof buf, fmt, *v);
        wg_label_in(ctx, r, buf, WG_TEXT_CENTER, ctx->style.text);
    }

    // float compare is fine here, we only care that a drag actually moved it.
    return *v != old;
}

int wg_slider_float(wg_context *ctx, wg_layout *l, const char *label,
                    float *v, float vmin, float vmax) {
    // label gets the left third, the track the rest. keeps panels tidy.
    wg_rect row = wg_layout_row(l, ctx, 0);
    wg_rect lr  = wg_rect_make(row.x, row.y, row.w * 0.34f, row.h);
    wg_rect tr  = wg_rect_make(row.x + row.w * 0.36f, row.y, row.w * 0.64f, row.h);

    wg_label_in(ctx, lr, label, WG_TEXT_LEFT, ctx->style.text_dim);
    wg_id id = wg_gen_id(ctx, label);
    return wg_slider_rect(ctx, id, tr, v, vmin, vmax, "%.2f");
}

int wg_slider_int(wg_context *ctx, wg_layout *l, const char *label,
                  int *v, int vmin, int vmax) {
    if (!v) return 0;
    float fv = (float)*v;
    wg_rect row = wg_layout_row(l, ctx, 0);
    wg_rect lr  = wg_rect_make(row.x, row.y, row.w * 0.34f, row.h);
    wg_rect tr  = wg_rect_make(row.x + row.w * 0.36f, row.y, row.w * 0.64f, row.h);

    wg_label_in(ctx, lr, label, WG_TEXT_LEFT, ctx->style.text_dim);
    wg_id id = wg_gen_id(ctx, label);
    wg_slider_rect(ctx, id, tr, &fv, (float)vmin, (float)vmax, NULL);

    int nv = (int)lroundf(fv);
    if (nv < vmin) nv = vmin;
    if (nv > vmax) nv = vmax;

    // draw the snapped integer over the track (the float slider hid its own).
    char buf[24];
    snprintf(buf, sizeof buf, "%d", nv);
    wg_label_in(ctx, tr, buf, WG_TEXT_CENTER, ctx->style.text);

    int changed = (nv != *v);
    *v = nv;
    return changed;
}

int wg_drag_int(wg_context *ctx, wg_layout *l, const char *label,
                int *v, float speed) {
    if (!v) return 0;
    wg_rect row = wg_layout_row(l, ctx, 0);
    wg_rect lr  = wg_rect_make(row.x, row.y, row.w * 0.34f, row.h);
    wg_rect dr  = wg_rect_make(row.x + row.w * 0.36f, row.y, row.w * 0.64f, row.h);

    wg_label_in(ctx, lr, label, WG_TEXT_LEFT, ctx->style.text_dim);

    wg_id id = wg_gen_id(ctx, label);
    int hovered = 0, held = 0;
    wg_behavior(ctx, id, dr, &hovered, &held);

    int old = *v;
    if (held) {
        // relative scrub: accumulate mouse_dx into the value. fractional remainder
        // is dropped each frame which can feel sticky at very low speed, but for
        // integer scrubbing it's fine and avoids needing per-widget float state.
        *v += (int)(ctx->input.mouse_dx * (speed <= 0.0f ? 1.0f : speed));
    }

    wg_rgba bg = held ? ctx->style.widget_active
                      : (hovered ? ctx->style.widget_hover : ctx->style.widget_bg);
    wg_draw_rect(&ctx->draw, dr, bg);
    wg_draw_border(&ctx->draw, dr, ctx->style.widget_border, ctx->style.border_thick);

    char buf[24];
    snprintf(buf, sizeof buf, "%d", *v);
    wg_label_in(ctx, dr, buf, WG_TEXT_CENTER, ctx->style.text);
    return *v != old;
}

void wg_progress(wg_context *ctx, wg_layout *l, float t01, const char *label) {
    wg_rect r = wg_layout_row(l, ctx, 0);
    t01 = clampf(t01, 0.0f, 1.0f);

    wg_draw_rect(&ctx->draw, r, ctx->style.widget_bg);
    wg_rect fill = wg_rect_make(r.x, r.y, r.w * t01, r.h);
    wg_draw_rect(&ctx->draw, fill, ctx->style.accent);
    wg_draw_border(&ctx->draw, r, ctx->style.widget_border, ctx->style.border_thick);

    if (label)
        wg_label_in(ctx, r, label, WG_TEXT_CENTER, ctx->style.text);
}
