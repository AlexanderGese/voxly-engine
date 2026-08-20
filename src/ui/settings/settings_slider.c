#include "settings_slider.h"

#include <string.h>

// map a cursor x to a 0..1 position within [track.x, track.x+track.w], leaving a
// grab-width margin so the handle center can reach both extremes.
static float cursor_to_norm(wg_rect track, float grab, float mx) {
    float lo = track.x + grab;
    float hi = track.x + track.w - grab;
    if (hi <= lo) return 0.0f;
    float t = (mx - lo) / (hi - lo);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t;
}

int settings_slider_track(wg_context *ctx, wg_id id, wg_rect track,
                          settings_value *v, const char *readout) {
    const wg_style *st = &ctx->style;
    float grab = st->slider_grab > 0 ? st->slider_grab : 6.0f;
    int changed = 0;

    int hov = 0, held = 0;
    wg_behavior(ctx, id, track, &hov, &held);

    // while held, drive the value from the cursor every frame. this is the
    // absolute-drag behavior; we don't bother with grab offsets for a settings
    // slider where jump-to-cursor is the expectation.
    if (wg_is_active(ctx, id)) {
        float t = cursor_to_norm(track, grab, ctx->input.mouse_x);
        changed |= settings_value_set_norm(v, t);
        held = 1;
    }

    // wheel over the track nudges by one step. nice for fine volume tweaks.
    if (hov && ctx->input.scroll != 0.0f) {
        int dir = ctx->input.scroll > 0 ? 1 : -1;
        changed |= settings_value_step(v, dir);
    }

    // chrome: recessed track, accent fill up to the current value, then the grab.
    float mid_y = track.y + track.h * 0.5f;
    wg_rect groove = wg_rect_make(track.x, mid_y - 2.0f, track.w, 4.0f);
    wg_draw_rect(&ctx->draw, groove, st->widget_bg);

    float t = settings_value_norm(v);
    float fill_w = (track.w - 2 * grab) * t + grab;
    wg_rect fill = wg_rect_make(track.x, mid_y - 2.0f, fill_w, 4.0f);
    wg_draw_rect(&ctx->draw, fill, held ? st->accent : st->accent_dim);

    // handle
    float hx = track.x + grab + (track.w - 2 * grab) * t;
    wg_rect handle = wg_rect_make(hx - grab, track.y + 2.0f,
                                  grab * 2.0f, track.h - 4.0f);
    wg_rgba hc = held ? st->widget_active
               : hov ? st->widget_hover
               : st->widget_border;
    wg_draw_rect(&ctx->draw, handle, hc);
    wg_draw_border(&ctx->draw, handle, st->widget_border, st->border_thick);

    if (readout && readout[0]) {
        float scale = st->font_scale;
        float tw = (float)strlen(readout) * 6.0f * scale;
        float tx = track.x + track.w - tw - 2.0f;
        float ty = track.y + (track.h - 8.0f * scale) * 0.5f;
        wg_draw_text(&ctx->draw, tx, ty, readout, scale, st->text_dim);
    }
    return changed;
}

int settings_toggle_pill(wg_context *ctx, wg_id id, wg_rect r, settings_value *v) {
    const wg_style *st = &ctx->style;
    int hov = 0, held = 0;
    int hit = wg_behavior(ctx, id, r, &hov, &held);
    int changed = 0;
    if (hit) changed = settings_value_cycle(v, 1);

    int on = v->i != 0;
    // the pill: track tinted by state, knob slid left/right.
    wg_rgba track_c = on ? st->accent_dim : st->widget_bg;
    if (hov) track_c = on ? st->accent : st->widget_hover;
    wg_draw_rect(&ctx->draw, r, track_c);
    wg_draw_border(&ctx->draw, r, st->widget_border, st->border_thick);

    float knob_w = r.w * 0.45f;
    float kx = on ? (r.x + r.w - knob_w - 2.0f) : (r.x + 2.0f);
    wg_rect knob = wg_rect_make(kx, r.y + 2.0f, knob_w, r.h - 4.0f);
    wg_draw_rect(&ctx->draw, knob, held ? st->widget_active : st->widget_border);

    const char *txt = on ? "on" : "off";
    float scale = st->font_scale;
    float tw = (float)strlen(txt) * 6.0f * scale;
    float tx = on ? (r.x + 6.0f) : (r.x + r.w - tw - 6.0f);
    float ty = r.y + (r.h - 8.0f * scale) * 0.5f;
    wg_draw_text(&ctx->draw, tx, ty, txt, scale, st->text);
    return changed;
}

int settings_cycler(wg_context *ctx, wg_id id, wg_rect r,
                    const char *label, settings_value *v) {
    const wg_style *st = &ctx->style;
    (void)v;   // the cycler reports intent; the layout applies it to the value

    // carve left/right arrow zones; the middle shows the current choice.
    float arrow_w = r.h;   // square arrow buttons
    wg_rect left  = wg_rect_make(r.x, r.y, arrow_w, r.h);
    wg_rect right = wg_rect_make(r.x + r.w - arrow_w, r.y, arrow_w, r.h);
    wg_rect mid   = wg_rect_make(r.x + arrow_w, r.y,
                                 r.w - 2 * arrow_w, r.h);

    int dir = 0;
    {
        wg_id lid = wg_gen_id_n(ctx, "settings.cyc.l", (int)(id & 0x7fff));
        int hov = 0, held = 0;
        if (wg_behavior(ctx, lid, left, &hov, &held)) dir = -1;
        wg_draw_rect(&ctx->draw, left, hov ? st->widget_hover : st->widget_bg);
        wg_draw_text(&ctx->draw, left.x + arrow_w * 0.35f,
                     left.y + (left.h - 8.0f * st->font_scale) * 0.5f,
                     "<", st->font_scale, st->text);
    }
    {
        wg_id rid = wg_gen_id_n(ctx, "settings.cyc.r", (int)(id & 0x7fff));
        int hov = 0, held = 0;
        if (wg_behavior(ctx, rid, right, &hov, &held)) dir = 1;
        wg_draw_rect(&ctx->draw, right, hov ? st->widget_hover : st->widget_bg);
        wg_draw_text(&ctx->draw, right.x + arrow_w * 0.35f,
                     right.y + (right.h - 8.0f * st->font_scale) * 0.5f,
                     ">", st->font_scale, st->text);
    }

    // clicking the middle also advances (forward), the common shortcut.
    {
        int hov = 0, held = 0;
        if (wg_behavior(ctx, id, mid, &hov, &held) && dir == 0) dir = 1;
        wg_draw_rect(&ctx->draw, mid, hov ? st->widget_hover : st->widget_bg);
    }
    wg_draw_border(&ctx->draw, r, st->widget_border, st->border_thick);

    if (label) {
        float scale = st->font_scale;
        float tw = (float)strlen(label) * 6.0f * scale;
        float tx = mid.x + (mid.w - tw) * 0.5f;
        float ty = mid.y + (mid.h - 8.0f * scale) * 0.5f;
        wg_draw_text(&ctx->draw, tx, ty, label, scale, st->text);
    }
    return dir;
}
