#include "settings_menu.h"
#include "settings_panel.h"

#include <string.h>

void settings_menu_init(settings_menu *sm) {
    memset(sm, 0, sizeof *sm);
    settings_model_init(&sm->model);
    settings_tabs_init(&sm->tabs);
    for (int i = 0; i < SETTINGS_TAB_COUNT; i++)
        settings_layout_init(&sm->layouts[i]);
    sm->title = "settings";
    sm->open = 0;
}

void settings_menu_open(settings_menu *sm) {
    sm->open = 1;
    // truth-first: live is whatever the engine is running, snapshot it so revert
    // and the dirty diff both compare against reality, then mirror into work.
    settings_model_snapshot(&sm->model);
    memcpy(sm->model.work, sm->model.live, sizeof sm->model.work);
    settings_model_refresh(&sm->model);
    settings_tabs_set(&sm->tabs, SETTINGS_TAB_VIDEO);
    for (int i = 0; i < SETTINGS_TAB_COUNT; i++)
        settings_layout_reset(&sm->layouts[i]);
}

void settings_menu_close(settings_menu *sm) {
    sm->open = 0;
    settings_model_revert(&sm->model);   // drop uncommitted edits
}

// a footer button. returns 1 when clicked. `enabled` greys it out and eats the
// click when false (apply when clean, revert when nothing to revert, etc).
static int footer_button(wg_context *ctx, wg_rect r, const char *label,
                         int enabled, int salt) {
    const wg_style *st = &ctx->style;
    wg_id id = wg_gen_id_n(ctx, "settings.footer", salt);
    int hov = 0, held = 0, hit = 0;
    if (enabled) {
        hit = wg_behavior(ctx, id, r, &hov, &held);
    }
    wg_rgba bg = !enabled ? st->widget_bg
               : held ? st->widget_active
               : hov ? st->widget_hover
               : st->widget_bg;
    wg_draw_rect(&ctx->draw, r, bg);
    wg_draw_border(&ctx->draw, r, st->widget_border, st->border_thick);

    wg_rgba tc = enabled ? st->text : st->text_disabled;
    float scale = st->font_scale;
    float tw = (float)strlen(label) * 6.0f * scale;
    float tx = r.x + (r.w - tw) * 0.5f;
    float ty = r.y + (r.h - 8.0f * scale) * 0.5f;
    wg_draw_text(&ctx->draw, tx, ty, label, scale, tc);
    return hit;
}

settings_result settings_menu_build(settings_menu *sm, wg_context *ctx,
                                    wg_rect area) {
    const wg_style *st = &ctx->style;
    settings_result result = SETTINGS_RESULT_NONE;

    // panel chrome behind everything.
    wg_draw_panel(&ctx->draw, area, st->panel_bg, st->panel_border);
    wg_rect inner = wg_rect_inset(area, st->pad);

    // title bar.
    wg_rect title_bar = wg_rect_cut(&inner, 2 /*top*/, st->row_height);
    wg_draw_rect(&ctx->draw, title_bar, st->title_bg);
    {
        float scale = st->font_scale * 1.1f;
        float ty = title_bar.y + (title_bar.h - 8.0f * scale) * 0.5f;
        wg_draw_text(&ctx->draw, title_bar.x + st->pad, ty,
                     sm->title ? sm->title : "settings", scale, st->title_text);
    }
    // a hair of breathing room between the title and the tab strip.
    settings_panel_spacer(&inner, st->spacing);

    // tab strip.
    wg_rect tab_bar = wg_rect_cut(&inner, 2 /*top*/, st->row_height);
    settings_tab active = settings_tabs_build(&sm->tabs, ctx, tab_bar);

    // keyboard: tab / shift-tab walk the strip, escape closes. we read these at
    // the panel level so they work regardless of which control is hot.
    if (wg_input_key_pressed(&ctx->input, WG_KEY_TAB)) {
        int dir = ctx->input.shift ? -1 : 1;
        if (settings_tabs_next(&sm->tabs, dir)) {
            active = sm->tabs.active;
            settings_layout_reset(&sm->layouts[active]);
        }
    }
    if (wg_input_key_pressed(&ctx->input, WG_KEY_ESCAPE)) {
        // escape behaves like back: bail without committing.
        settings_menu_close(sm);
        return SETTINGS_RESULT_CLOSE;
    }

    // footer strip carved off the bottom before the content takes the middle.
    float foot_h = st->row_height + st->spacing;
    wg_rect footer = wg_rect_cut(&inner, 3 /*bottom*/, foot_h);
    wg_rect content = inner;

    // the active tab's option list.
    int changed = settings_layout_build(&sm->layouts[active], ctx, content,
                                        &sm->model, active);
    if (changed) settings_model_refresh(&sm->model);

    // footer: revert | reset defaults | apply. split into three equal cells.
    float gap = st->spacing;
    float cell_w = (footer.w - 2 * gap) / 3.0f;
    wg_rect c_revert  = wg_rect_make(footer.x, footer.y, cell_w, footer.h - gap);
    wg_rect c_reset   = wg_rect_make(footer.x + cell_w + gap, footer.y,
                                     cell_w, footer.h - gap);
    wg_rect c_apply   = wg_rect_make(footer.x + 2 * (cell_w + gap), footer.y,
                                     cell_w, footer.h - gap);

    int dirty = sm->model.dirty_count > 0;

    if (footer_button(ctx, c_revert, "revert", dirty, 1))
        settings_model_revert(&sm->model);

    if (footer_button(ctx, c_reset, "defaults", 1, 2))
        settings_model_defaults(&sm->model);

    if (footer_button(ctx, c_apply, "apply", dirty, 3)) {
        settings_model_apply(&sm->model);
    }

    // the apply machine raises a one-shot APPLIED edge; surface it to the host so
    // it knows to re-read live exactly once.
    if (settings_model_take_applied(&sm->model))
        result = SETTINGS_RESULT_APPLIED;

    return result;
}

const settings_value *settings_menu_value(const settings_menu *sm,
                                          settings_id id) {
    return settings_model_live(&sm->model, id);
}
