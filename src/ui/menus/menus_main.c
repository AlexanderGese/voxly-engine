#include "menus_build.h"
#include "menus_manager.h"
#include "menus_controls.h"
#include "../widgets/widgets_label.h"
static menus_action main_build(struct menus_manager *m, void *state,
                               wg_context *ctx, wg_rect area) {
    (void)state;

    wg_layout l;
    wg_layout_begin(&l, ctx, area);

    // big wordmark up top. the engine's called voxl; lean into it.
    wg_rect brand = wg_layout_row(&l, ctx, ctx->style.row_height * 1.8f);
    wg_label_in(ctx, brand, "voxl", WG_TEXT_CENTER, ctx->style.accent);
    wg_layout_gap(&l, ctx->style.spacing * 2.0f);

    menus_action act = MENUS_ACT_NONE;
    menus_nav *nav = &menus_stack_top(&m->stack)->nav;

    // "continue" only makes sense if a world is already loaded behind us. on the
    // title screen that's usually false, but the same screen doubles as the
    // root when you back all the way out of pause, so honor the flag.
    if (m->in_world) {
        if (menus_ctl_button(ctx, &l, nav, "continue", 1))
            act = MENUS_ACT_RESUME;
    }

    if (menus_ctl_button(ctx, &l, nav, "new world", 1))
        act = MENUS_ACT_NEW_WORLD;

    if (menus_ctl_button(ctx, &l, nav, "load world", 1))
        act = MENUS_ACT_LOAD_WORLD;

    wg_layout_gap(&l, ctx->style.spacing);

    if (menus_ctl_button(ctx, &l, nav, "settings", 1))
        act = MENUS_ACT_OPEN_SETTINGS;

    if (menus_ctl_button(ctx, &l, nav, "controls", 1))
        act = MENUS_ACT_OPEN_KEYBINDS;

    wg_layout_gap(&l, ctx->style.spacing * 2.0f);

    if (menus_ctl_button(ctx, &l, nav, "quit", 1))
        act = MENUS_ACT_QUIT;

    // footer hint, dim, bottom of the panel.
    wg_rect foot = wg_rect_make(area.x, area.y + area.h - WG_GLYPH_H * 1.5f,
                                area.w, WG_GLYPH_H * 1.5f);
    wg_label_in(ctx, foot, "arrows + enter, or click",
                WG_TEXT_CENTER, ctx->style.text_disabled);

    return act;
}

static const menus_screen_vtbl g_main_vtbl = {
    .on_enter = NULL,
    .on_leave = NULL,
    .build    = main_build,
};
