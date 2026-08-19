#include "menus_build.h"
#include "menus_manager.h"
#include "menus_controls.h"
#include <stdlib.h>
#include "../widgets/widgets_label.h"
// the settings screen edits the manager's working copy (m->edit) and only raises
// APPLY when the user commits. back without applying discards by re-snapshotting
// from the live struct on the way out.
//
typedef struct {
    float scroll;        // pixels scrolled, >= 0
} settings_state;
static void settings_enter(struct menus_manager *m, void *state) {
    settings_state *ss = state;
    ss->scroll = 0.0f;
    // re-mirror live settings into the working copy so opening settings always
    // starts from the truth, even if a previous visit left edits uncommitted.
    if (m->settings) m->edit = *m->settings;
    m->dirty_settings = 0;
}

static menus_action settings_build(struct menus_manager *m, void *state,
                                   wg_context *ctx, wg_rect area) {
    settings_state *ss = state;
menus_settings *e  = &m->edit;
menus_nav *nav = &menus_stack_top(&m->stack)->nav;
menus_action act = MENUS_ACT_NONE;
int changed = 0;
wg_rect content = area;
float foot_h = ctx->style.row_height + ctx->style.spacing;
wg_rect footer = wg_rect_cut(&content, 3 /*bottom*/, foot_h);
if (wg_input_over(&ctx->input, content) && ctx->input.scroll != 0.0f) {
        ss->scroll -= ctx->input.scroll * ctx->style.row_height * 1.5f;
        if (ss->scroll < 0.0f) ss->scroll = 0.0f;
    }

    // clip the list so scrolled-out rows don't paint over the footer/title.
    wg_draw_push_clip(&ctx->draw, content);
wg_rect laid = content;
laid.y -= ss->scroll;
laid.h += ss->scroll;
wg_layout l;
wg_layout_begin(&l, ctx, laid);
menus_ctl_header(ctx, &l, "audio");
changed |= menus_ctl_slider(ctx, &l, nav, "master volume",
                                &e->master_volume, 0.0f, 1.0f, 0.05f);
changed |= menus_ctl_slider(ctx, &l, nav, "music volume",
                                &e->music_volume, 0.0f, 1.0f, 0.05f);
menus_ctl_header(ctx, &l, "controls");
changed |= menus_ctl_slider(ctx, &l, nav, "mouse sensitivity",
                                &e->mouse_sens, 0.02f, 0.5f, 0.01f);
changed |= menus_ctl_toggle(ctx, &l, nav, "invert y", &e->invert_y);
menus_ctl_header(ctx, &l, "video");
changed |= menus_ctl_spinner(ctx, &l, nav, "render distance",
                                 &e->render_distance, 2, 16);
changed |= menus_ctl_spinner(ctx, &l, nav, "field of view",
                                 &e->fov, 60, 110);
changed |= menus_ctl_slider(ctx, &l, nav, "gamma",
                                &e->gamma, 0.5f, 2.0f, 0.05f);
changed |= menus_ctl_toggle(ctx, &l, nav, "vsync",   &e->vsync);
changed |= menus_ctl_toggle(ctx, &l, nav, "show fps", &e->show_fps);
wg_draw_pop_clip(&ctx->draw);
if (changed) {
        menus_settings_clamp(e);
        m->dirty_settings = m->settings
            ? menus_settings_differs(e, m->settings) : 1;
    }

    // footer: apply (live only when dirty) + back, split into two cells.
    wg_layout fl;
wg_layout_begin(&fl, ctx, footer);
wg_layout_begin_row(&fl, ctx, 2, 0);
wg_rect c_apply = wg_layout_cell(&fl, ctx);
wg_rect c_back  = wg_layout_cell(&fl, ctx);
wg_layout_end_row(&fl);
{
        wg_id id = wg_gen_id(ctx, "settings.apply");
        int focused = menus_nav_item(nav, id, MENUS_NO_ITEM);
        menus_nav_dir intent = menus_nav_take_intent(nav, id);
        int hov = 0, held = 0, hit = 0;
        if (m->dirty_settings) {
            hit = wg_behavior(ctx, id, c_apply, &hov, &held);
            if (intent == MENUS_NAV_ACTIVATE) hit = 1;
        }
        wg_rgba bg = !m->dirty_settings ? ctx->style.widget_bg
                   : held ? ctx->style.widget_active
                   : (hov || focused) ? ctx->style.widget_hover
                   : ctx->style.accent_dim;
        wg_draw_rect(&ctx->draw, c_apply, bg);
        wg_draw_border(&ctx->draw, c_apply, ctx->style.widget_border,
                       ctx->style.border_thick);
        wg_rgba tc = m->dirty_settings ? ctx->style.text
                                       : ctx->style.text_disabled;
        wg_label_in(ctx, c_apply, "apply", WG_TEXT_CENTER, tc);
        if (hit) act = MENUS_ACT_APPLY_SETTINGS;
    }

    // back
    {
        wg_id id = wg_gen_id(ctx, "settings.back");
int focused = menus_nav_item(nav, id, MENUS_NO_ITEM);
menus_nav_dir intent = menus_nav_take_intent(nav, id);
int hov = 0, held = 0;
int hit = wg_behavior(ctx, id, c_back, &hov, &held);
if (intent == MENUS_NAV_ACTIVATE) hit = 1;
wg_rgba bg = held ? ctx->style.widget_active
                   : (hov || focused) ? ctx->style.widget_hover
                   : ctx->style.widget_bg;
wg_draw_rect(&ctx->draw, c_back, bg);
wg_draw_border(&ctx->draw, c_back, ctx->style.widget_border,
                       ctx->style.border_thick);
wg_label_in(ctx, c_back, "back", WG_TEXT_CENTER, ctx->style.text);
if (hit) act = MENUS_ACT_BACK;
}

    return act;
}

static void settings_leave(struct menus_manager *m, void *state, int destroyed) {
    (void)state;
    // leaving without applying throws away uncommitted edits — re-mirror live so
    // a future visit doesn't inherit stale changes.
    if (destroyed && m->settings) {
        m->edit = *m->settings;
        m->dirty_settings = 0;
    }
}

static const menus_screen_vtbl g_settings_vtbl = {
    .on_enter = settings_enter,
    .on_leave = settings_leave,
    .build    = settings_build,
};
menus_screen menus_make_settings(struct menus_manager *m) {
    (void)m;
    settings_state *ss = calloc(1, sizeof *ss);
    menus_screen s;
    menus_screen_init(&s, MENUS_SCREEN_SETTINGS, &g_settings_vtbl,
                      ss, 1, "settings");
    return s;
}
