#include "menus_build.h"
#include "menus_manager.h"
#include "menus_controls.h"
#include "menus_keybind.h"
#include <stdlib.h>
#include <stdio.h>
#include "../widgets/widgets_label.h"
typedef struct {
    float scroll;
} keybinds_state;
static void kb_enter(struct menus_manager *m, void *state) {
    keybinds_state *ks = state;
    ks->scroll = 0.0f;
    // re-snapshot for the reset button every time we open the screen.
    menus_kb_init(&m->kb, m->binds);
}

static menus_action kb_build(struct menus_manager *m, void *state,
                             wg_context *ctx, wg_rect area) {
    keybinds_state *ks = state;
menus_nav *nav = &menus_stack_top(&m->stack)->nav;
menus_kb_state *kb = &m->kb;
menus_action act = MENUS_ACT_NONE;
if (kb->phase == MENUS_KB_ARMED &&
        wg_input_key_pressed(&ctx->input, WG_KEY_ESCAPE)) {
        menus_kb_cancel(kb);
        ctx->want_keys = 1;
    }

    // footer for reset/back, like settings.
    wg_rect content = area;
float foot_h = ctx->style.row_height + ctx->style.spacing;
wg_rect footer = wg_rect_cut(&content, 3, foot_h);
if (wg_input_over(&ctx->input, content) && ctx->input.scroll != 0.0f) {
        ks->scroll -= ctx->input.scroll * ctx->style.row_height * 1.5f;
        if (ks->scroll < 0.0f) ks->scroll = 0.0f;
    }

    wg_draw_push_clip(&ctx->draw, content);
wg_rect laid = content;
laid.y -= ks->scroll;
laid.h += ks->scroll;
wg_layout l;
wg_layout_begin(&l, ctx, laid);
const menus_kb_action *actions = menus_kb_actions();
for (int i = 0;
i < MENUS_KB_ACTIONS;
i++) {
        wg_rect row = wg_layout_row(&l, ctx, 0);

        // label fills the left half.
        wg_rect label = row;
        label.w = row.w * 0.55f;
        wg_label_in(ctx, label, actions[i].name, WG_TEXT_LEFT, ctx->style.text);

        // binding button on the right half.
        wg_rect btn = row;
        btn.x = row.x + row.w * 0.58f;
        btn.w = row.w - row.w * 0.58f;

        wg_id id = wg_gen_id_n(ctx, "kbrow", i);
        int focused = menus_nav_item(nav, id, i);
        menus_nav_dir intent = menus_nav_take_intent(nav, id);

        int armed_here = (kb->phase == MENUS_KB_ARMED && kb->armed_action == i);

        int hov = 0, held = 0;
        int hit = wg_behavior(ctx, id, btn, &hov, &held);
        if (intent == MENUS_NAV_ACTIVATE) hit = 1;
        if (hit) menus_kb_arm(kb, i);

        // surface color: armed rows glow with the accent so it's obvious which
        // one is listening.
        wg_rgba bg;
        if (armed_here)            bg = ctx->style.accent_dim;
        else if (held)             bg = ctx->style.widget_active;
        else if (hov || focused)   bg = ctx->style.widget_hover;
        else                       bg = ctx->style.widget_bg;
        wg_draw_rect(&ctx->draw, btn, bg);
        wg_draw_border(&ctx->draw, btn, ctx->style.widget_border,
                       ctx->style.border_thick);
        if (focused && !armed_here) {
            wg_draw_border(&ctx->draw, wg_rect_inset(btn, 2.0f),
                           ctx->style.accent, ctx->style.border_thick);
        }

        char cap[24];
        if (armed_here) {
            snprintf(cap, sizeof cap, "press a key...");
        } else {
            menus_kb_key_name(menus_kb_get(m->binds, i), cap, sizeof cap);
        }
        wg_label_in(ctx, btn, cap, WG_TEXT_CENTER,
                    armed_here ? ctx->style.title_text : ctx->style.text);

        // flag a conflict on the row that just got a colliding key.
        if (kb->conflict && kb->conflict_action == i) {
            wg_rect mark = btn;
            mark.x = btn.x - 14.0f; mark.w = 12.0f;
            wg_label_in(ctx, mark, "!", WG_TEXT_CENTER,
                        wg_rgba_make(230, 90, 70, 255));
        }
    }

    wg_draw_pop_clip(&ctx->draw);
if (kb->conflict && kb->conflict_action >= 0) {
        wg_rect warn = wg_rect_make(content.x,
                                    content.y + content.h - WG_GLYPH_H * 1.6f,
                                    content.w, WG_GLYPH_H * 1.6f);
        char msg[64];
        snprintf(msg, sizeof msg, "also bound to \"%s\"",
                 actions[kb->conflict_action].name);
        wg_label_in(ctx, warn, msg, WG_TEXT_CENTER,
                    wg_rgba_make(230, 90, 70, 255));
    }

    // footer: reset + back.
    wg_layout fl;
wg_layout_begin(&fl, ctx, footer);
wg_layout_begin_row(&fl, ctx, 2, 0);
wg_rect c_reset = wg_layout_cell(&fl, ctx);
wg_rect c_back  = wg_layout_cell(&fl, ctx);
wg_layout_end_row(&fl);
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
instead we cancel directly and let the
    // manager's escape fold see we're no longer armed next frame. nothing to do.
    return act;
;
