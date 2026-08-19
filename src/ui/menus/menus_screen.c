#include "menus_screen.h"

#include <string.h>

void menus_screen_init(menus_screen *s, menus_screen_id id,
                       const menus_screen_vtbl *vt, void *state,
                       int owns_state, const char *title) {
    s->id         = id;
    s->vt         = vt;
    s->state      = state;
    s->owns_state = owns_state;
    menus_nav_init(&s->nav);

    // copy the title, truncating defensively. nobody should be naming a screen
    // 48 chars but the field is fixed so clamp anyway.
    if (title) {
        size_t n = strlen(title);
        if (n >= sizeof(s->title)) n = sizeof(s->title) - 1;
        memcpy(s->title, title, n);
        s->title[n] = '\0';
    } else {
        s->title[0] = '\0';
    }
}

void menus_screen_enter(struct menus_manager *m, menus_screen *s) {
    // fresh focus ring every time we land on a screen — the ring is rebuilt each
    // frame anyway, this just clears the remembered index so the first item is
    // highlighted instead of whatever happened to be focused last time.
    menus_nav_reset(&s->nav);
    if (s->vt && s->vt->on_enter) s->vt->on_enter(m, s->state);
}

void menus_screen_leave(struct menus_manager *m, menus_screen *s, int destroyed) {
    if (s->vt && s->vt->on_leave) s->vt->on_leave(m, s->state, destroyed);
}

menus_action menus_screen_build(struct menus_manager *m, menus_screen *s,
                                wg_context *ctx, wg_rect area) {
    if (!s->vt || !s->vt->build) return MENUS_ACT_NONE;
    return s->vt->build(m, s->state, ctx, area);
}
