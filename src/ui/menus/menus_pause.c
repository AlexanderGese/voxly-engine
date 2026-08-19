#include "menus_build.h"
#include "menus_manager.h"
#include "menus_controls.h"

#include "../widgets/widgets_label.h"

// the in-world pause menu. esc opens it, esc again resumes (handled by the
// manager folding CANCEL -> BACK, which pops the root and closes the system).
// stateless like the main screen.

static menus_action pause_build(struct menus_manager *m, void *state,
                                wg_context *ctx, wg_rect area) {
    (void)state;

    menus_nav *nav = &menus_stack_top(&m->stack)->nav;
    menus_action act = MENUS_ACT_NONE;

    wg_layout l;
    wg_layout_begin(&l, ctx, area);

    wg_rect head = wg_layout_row(&l, ctx, ctx->style.row_height * 1.3f);
    wg_label_in(ctx, head, "paused", WG_TEXT_CENTER, ctx->style.text);
    wg_layout_gap(&l, ctx->style.spacing * 1.5f);

    if (menus_ctl_button(ctx, &l, nav, "resume", 1))
        act = MENUS_ACT_RESUME;

    if (menus_ctl_button(ctx, &l, nav, "settings", 1))
        act = MENUS_ACT_OPEN_SETTINGS;

    if (menus_ctl_button(ctx, &l, nav, "controls", 1))
        act = MENUS_ACT_OPEN_KEYBINDS;

    wg_layout_gap(&l, ctx->style.spacing);

    // saving doesn't leave the menu — it raises SAVE and the host writes the
    // region files, then we stay put so the player can keep tweaking or resume.
    if (menus_ctl_button(ctx, &l, nav, "save world", 1))
        act = MENUS_ACT_SAVE;

    wg_layout_gap(&l, ctx->style.spacing * 2.0f);

    // save & quit is two intents; we surface QUIT and trust the host to flush a
    // save on the way out (it already does on normal exit). a dedicated combined
    // action wasn't worth another enum.
    if (menus_ctl_button(ctx, &l, nav, "save & quit to title", 1))
        act = MENUS_ACT_QUIT;

    return act;
}

static const menus_screen_vtbl g_pause_vtbl = {
    .on_enter = NULL,
    .on_leave = NULL,
    .build    = pause_build,
};

menus_screen menus_make_pause(struct menus_manager *m) {
    (void)m;
    menus_screen s;
    menus_screen_init(&s, MENUS_SCREEN_PAUSE, &g_pause_vtbl,
                      NULL, 0, "game menu");
    return s;
}
