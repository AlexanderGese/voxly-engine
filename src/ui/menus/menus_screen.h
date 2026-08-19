#ifndef UI_MENUS_SCREEN_H
#define UI_MENUS_SCREEN_H
// one screen = one buildable page (main / pause / settings / keybinds). a screen
// is just a small vtable plus a blob of opaque per-screen state. the manager owns
// the stack of these and calls build() once per frame for whatever's on top.
//
// screens never poke gl or the game directly. build() walks the widget layer and
// returns a menus_action; the manager folds that up to the host. this is the same
// "dependency arrow points one way" rule the rest of ui/ follows.
#include "menus_types.h"
#include "menus_nav.h"
#include "../widgets/widgets_context.h"  // wg_context + wg_rect, the im-mode spine
struct menus_manager;
// fwd, defined in menus_manager.h
// per-screen function table. any hook may be NULL; the manager null-checks before
// calling so a screen only fills in what it needs.
typedef struct {
    // called when the screen becomes the stack top (either freshly pushed or
    // re-exposed by a pop above it). good place to reset focus / re-snapshot.
    void (*on_enter)(struct menus_manager *m, void *state);

    // called when the screen stops being the top (pushed over, or popped off).
    // `destroyed` is 1 only on the pop-off case so the screen can free state.
    void (*on_leave)(struct menus_manager *m, void *state, int destroyed);

    // the meat: emit widgets for one frame, return an action. `area` is the
    // content rect the manager already centered/sized for us.
    menus_action (*build)(struct menus_manager *m, void *state,
                          wg_context *ctx, wg_rect area);
} menus_screen_vtbl;
// a live screen instance on the stack. state points at one of the concrete
// per-screen structs (allocated by the screen's make()).
typedef struct {
    menus_screen_id    id;
    const menus_screen_vtbl *vt;
    void              *state;     // owned; freed on pop if the vtbl said so
    int                owns_state;

    menus_nav          nav;       // focus ring, one per screen so back/forward
                                  // restores where you were.
    char               title[48]; // shown in the panel header
} menus_screen;
// helpers the concrete screens use to wire themselves up. these just fill the
// struct; the manager does the actual push.
void menus_screen_init(menus_screen *s, menus_screen_id id,
                       const menus_screen_vtbl *vt, void *state,
                       int owns_state, const char *title);
// dispatch helpers — null-safe wrappers around the vtbl pointers.
void         menus_screen_enter(struct menus_manager *m, menus_screen *s);
void         menus_screen_leave(struct menus_manager *m, menus_screen *s, int destroyed);
menus_action menus_screen_build(struct menus_manager *m, menus_screen *s,
                                wg_context *ctx, wg_rect area);
#endif
