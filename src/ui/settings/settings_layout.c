#include "settings_layout.h"
#include "settings_schema.h"
#include "settings_slider.h"
#include <string.h>
void settings_layout_init(settings_layout *sl) {
    memset(sl, 0, sizeof *sl);
}

void settings_layout_reset(settings_layout *sl) {
    sl->scroll = 0.0f;
}

// draw a label in the left gutter of a row, vertically centered. the widget
// draw_text anchors top-left so we do the centering ourselves.
static void label_left(wg_context *ctx, wg_rect row, const char *text,
                       wg_rgba color) {
    float scale = ctx->style.font_scale;
    float ty = row.y + (row.h - 8.0f * scale) * 0.5f;
    wg_draw_text(&ctx->draw, row.x + ctx->style.pad, ty, text, scale, color);
}

// the little "changed" pip drawn at the far left of a dirty row.
static void dirty_pip(wg_context *ctx, wg_rect row) {
    float s = 4.0f;
wg_rect pip = wg_rect_make(row.x + 2.0f, row.y + (row.h - s) * 0.5f, s, s);
wg_draw_rect(&ctx->draw, pip, ctx->style.accent);
}

// dispatch one option onto the right control. returns 1 if it changed the work
// value. `row` is the full row rect;
we split label/control here.
static int build_option(wg_context *ctx, wg_rect row, settings_model *m,
                        settings_id id, int field_dirty) {
    const settings_opt *opt = settings_schema_opt(id);
    settings_value *v = settings_model_work(m, id);
    const wg_style *st = &ctx->style;

    // left half: label. right ~55%: control. clamp so narrow panels still work.
    float ctrl_w = row.w * 0.5f;
    if (ctrl_w < 80.0f) ctrl_w = row.w * 0.6f;
    wg_rect ctrl = wg_rect_make(row.x + row.w - ctrl_w, row.y + 2.0f,
                                ctrl_w - st->pad, row.h - 4.0f);

    if (field_dirty) dirty_pip(ctx, row);
    label_left(ctx, row, settings_id_label(id),
               field_dirty ? st->text : st->text_dim);

    wg_id id_w = wg_gen_id_n(ctx, "settings.opt", (int)id);
    int changed = 0;

    switch (opt->kind) {
    case SETTINGS_OPT_BOOL:
        changed = settings_toggle_pill(ctx, id_w, ctrl, v);
        break;
    case SETTINGS_OPT_ENUM: {
        char buf[32];
        settings_value_format(v, opt->choices, opt->choice_count,
                              buf, sizeof buf);
        int dir = settings_cycler(ctx, id_w, ctrl, buf, v);
        if (dir != 0) changed = settings_value_cycle(v, dir);
        break;
    }
    case SETTINGS_OPT_FLOAT:
    case SETTINGS_OPT_INT:
    default: {
        char buf[32];
        settings_value_format(v, NULL, 0, buf, sizeof buf);
        changed = settings_slider_track(ctx, id_w, ctrl, v, buf);
        // keyboard nudge when this row is focused via the widget focus.
        if (wg_is_hot(ctx, id_w)) {
            if (wg_input_key_pressed(&ctx->input, WG_KEY_LEFT))
                changed |= settings_value_step(v, -1);
            if (wg_input_key_pressed(&ctx->input, WG_KEY_RIGHT))
                changed |= settings_value_step(v, 1);
        }
        break;
    }
    }
    return changed;
}

int settings_layout_build(settings_layout *sl, wg_context *ctx, wg_rect content,
                          settings_model *m, settings_tab tab) {
    const wg_style *st = &ctx->style;
settings_id ids[SETTINGS_ID_COUNT];
int n = settings_schema_tab_ids(tab, ids, SETTINGS_ID_COUNT);
float row_h = st->row_height;
float step  = row_h + st->spacing;
float content_h = (float)n * step;
sl->content_h = content_h;
if (max_scroll < 0.0f) max_scroll = 0.0f;
if (sl->scroll < 0.0f) sl->scroll = 0.0f;
if (sl->scroll > max_scroll) sl->scroll = max_scroll;
wg_draw_push_clip(&ctx->draw, content);
int changed = 0;
float y = content.y - sl->scroll;
for (int i = 0;
i < n;
}
