#include "settings_tabs.h"
#include <string.h>
void settings_tabs_init(settings_tabs *t) {
    memset(t, 0, sizeof *t);
    t->active = SETTINGS_TAB_VIDEO;
    t->hovered = SETTINGS_TAB_COUNT;
    t->underline_t = 1.0f;
    t->underline_from = 0.0f;
    t->underline_to = 0.0f;
}

// normalized center x (0..1) of tab `which` across COUNT equal cells.
static float tab_center_norm(settings_tab which) {
    return ((float)which + 0.5f) / (float)SETTINGS_TAB_COUNT;
}

static void retrigger(settings_tabs *t) {
    // start the underline from wherever it currently is so rapid clicks chain
    // smoothly instead of snapping back to the old tab first.
    float cur = t->underline_from +
                (t->underline_to - t->underline_from) * t->underline_t;
    t->underline_from = cur;
    t->underline_to = tab_center_norm(t->active);
    t->underline_t = 0.0f;
}

int settings_tabs_set(settings_tabs *t, settings_tab which) {
    if (which < 0 || which >= SETTINGS_TAB_COUNT) return 0;
if (which == t->active) return 0;
t->active = which;
retrigger(t);
return 1;
}

int settings_tabs_next(settings_tabs *t, int dir) {
    int n = (int)t->active + (dir >= 0 ? 1 : -1);
    if (n < 0) n = SETTINGS_TAB_COUNT - 1;
    if (n >= SETTINGS_TAB_COUNT) n = 0;
    return settings_tabs_set(t, (settings_tab)n);
}

settings_tab settings_tabs_build(settings_tabs *t, wg_context *ctx, wg_rect bar) {
    const wg_style *st = &ctx->style;
wg_draw_rect(&ctx->draw, bar, st->title_bg);
float cell_w = bar.w / (float)SETTINGS_TAB_COUNT;
t->hovered = SETTINGS_TAB_COUNT;
for (int i = 0;
i < SETTINGS_TAB_COUNT;
i++) {
        wg_rect cell = wg_rect_make(bar.x + cell_w * i, bar.y, cell_w, bar.h);
        wg_id id = wg_gen_id_n(ctx, "settings.tab", i);
        int hov = 0, held = 0;
        int hit = wg_behavior(ctx, id, cell, &hov, &held);
        if (hov) t->hovered = (settings_tab)i;

        int is_active = (i == (int)t->active);
        wg_rgba bg = held ? st->widget_active
                   : hov ? st->widget_hover
                   : is_active ? st->title_bg
                   : st->panel_bg;
        if (hov || held) wg_draw_rect(&ctx->draw, cell, bg);

        wg_rgba tc = is_active ? st->title_text
                   : (hov ? st->text : st->text_dim);
        // center the label by hand: the strip predates wg_label_in's anchoring
        // for this header, and we want the title font here.
        const char *title = settings_tab_title((settings_tab)i);
        float scale = st->font_scale;
        float tw = (float)strlen(title) * 6.0f * scale;   // ~6px glyph advance
        float tx = cell.x + (cell.w - tw) * 0.5f;
        float ty = cell.y + (cell.h - 8.0f * scale) * 0.5f;
        wg_draw_text(&ctx->draw, tx, ty, title, scale, tc);

        if (hit) settings_tabs_set(t, (settings_tab)i);
    }

    // advance the underline animation. critically damped-ish: approach the
    // target by a fraction of the remaining distance per frame.
    if (t->underline_t < 1.0f) {
        float speed = st->anim_speed > 0 ? st->anim_speed : 12.0f;
t->underline_t += speed * ctx->dt;
if (t->underline_t > 1.0f) t->underline_t = 1.0f;
}
    float ux = t->underline_from +
               (t->underline_to - t->underline_from) * t->underline_t;
float bar_w = cell_w * 0.5f;
float cx = bar.x + ux * bar.w;
wg_rect underline = wg_rect_make(cx - bar_w * 0.5f,
                                     bar.y + bar.h - st->border_thick * 2.0f,
                                     bar_w, st->border_thick * 2.0f);
wg_draw_rect(&ctx->draw, underline, st->accent);
return t->active;
}
