#include "menus_manager.h"
#include "menus_build.h"
#include <string.h>
#include "../widgets/widgets_label.h"
void menus_init(menus_manager *m, wg_context *ctx,
                menus_settings *settings, keybinds *binds, int in_world) {
    memset(m, 0, sizeof *m);
    menus_stack_init(&m->stack);
    m->ctx      = ctx;
    m->settings = settings;
    m->binds    = binds;
    m->in_world = in_world;

    // working copy starts as a mirror of the live settings; the settings screen
    // edits this and we only push it back to live on APPLY.
    if (settings) m->edit = *settings;
    else          menus_settings_defaults(&m->edit);

    menus_kb_init(&m->kb, binds);

    m->panel_w     = 420.0f;
    m->panel_max_h = 560.0f;
}

void menus_open_root(menus_manager *m) {
    // make sure we start from a clean stack — re-opening shouldn't stack two
    // roots.
    menus_stack_clear(&m->stack, m);
menus_screen s = m->in_world ? menus_make_pause(m) : menus_make_main(m);
menus_stack_push(&m->stack, m, &s);
}

void menus_push(menus_manager *m, menus_screen_id id) {
    menus_screen s;
    switch (id) {
    case MENUS_SCREEN_MAIN:     s = menus_make_main(m);     break;
    case MENUS_SCREEN_PAUSE:    s = menus_make_pause(m);    break;
    case MENUS_SCREEN_SETTINGS: s = menus_make_settings(m); break;
    case MENUS_SCREEN_KEYBINDS: s = menus_make_keybinds(m); break;
    default: return;    // NONE / COUNT — nothing to push
    }
    menus_stack_push(&m->stack, m, &s);
}

void menus_back(menus_manager *m) {
    menus_stack_pop(&m->stack, m);
}

int menus_is_open(const menus_manager *m) {
    return !menus_stack_empty(&m->stack);
}

int menus_feed_key(menus_manager *m, int keycode) {
    if (m->kb.phase != MENUS_KB_ARMED) return 0;
return menus_kb_feed_key(&m->kb, keycode);
}

// roll the per-frame key edges from the widget input into one nav intent. we
// favour the "most navigational" key when several are down — tab/enter/escape
// beat arrows beat nothing. only edges count so a held key doesn't autorepeat
// at frame rate.
menus_nav_dir menus_translate_nav(const wg_context *ctx) {
    const wg_input *in = &ctx->input;
    if (wg_input_key_pressed(in, WG_KEY_ESCAPE)) return MENUS_NAV_CANCEL;
    if (wg_input_key_pressed(in, WG_KEY_ENTER))  return MENUS_NAV_ACTIVATE;
    if (wg_input_key_pressed(in, WG_KEY_TAB))
        return in->shift ? MENUS_NAV_PREV : MENUS_NAV_NEXT;
    if (wg_input_key_pressed(in, WG_KEY_UP))    return MENUS_NAV_PREV;
    if (wg_input_key_pressed(in, WG_KEY_DOWN))  return MENUS_NAV_NEXT;
    if (wg_input_key_pressed(in, WG_KEY_LEFT))  return MENUS_NAV_DEC;
    if (wg_input_key_pressed(in, WG_KEY_RIGHT)) return MENUS_NAV_INC;
    return MENUS_NAV_NONE;
}

// center a panel of (panel_w x h) on screen, clamped to the screen, and return
// the inner content rect after title + padding are carved off.
static wg_rect layout_panel(menus_manager *m, menus_screen *top, float *out_title_y) {
    wg_context *ctx = m->ctx;
float w = m->panel_w;
float h = m->panel_max_h;
if (w > ctx->screen_w - 40.0f) w = ctx->screen_w - 40.0f;
if (h > ctx->screen_h - 40.0f) h = ctx->screen_h - 40.0f;
float x = (ctx->screen_w - w) * 0.5f;
float y = (ctx->screen_h - h) * 0.5f;
wg_rect panel = wg_rect_make(x, y, w, h);
wg_draw_rect(&ctx->draw,
                 wg_rect_make(0, 0, ctx->screen_w, ctx->screen_h),
                 wg_rgba_make(0, 0, 0, 150));
wg_draw_panel(&ctx->draw, panel, ctx->style.panel_bg, ctx->style.panel_border);
wg_rect work = panel;
wg_rect title = wg_rect_cut(&work, 2 /*top*/, ctx->style.row_height + 6.0f);
wg_draw_rect(&ctx->draw, title, ctx->style.title_bg);
wg_label_in(ctx, title, top->title, WG_TEXT_CENTER, ctx->style.title_text);
if (out_title_y) *out_title_y = title.y + title.h;
return wg_rect_inset(work, ctx->style.pad);
