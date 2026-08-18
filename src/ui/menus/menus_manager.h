#ifndef UI_MENUS_MANAGER_H
#define UI_MENUS_MANAGER_H
// the top-level menu manager. the game loop holds one of these and calls
// menus_update() once per frame when menus are open. the manager:
// - owns the screen stack and the shared widget context
// - centers a panel for the active screen and runs its build()
// - translates raw widget-layer keys into nav intents
// - folds the per-screen action into a single result for the host
//
// the host wires its real systems in by pointer (settings bag, keybinds, a couple
// of callbacks). the manager never includes game headers — it only knows the
// value types from menus_types.h and the input/keybinds struct.
#include "menus_stack.h"
#include "menus_types.h"
#include "menus_keybind.h"
#include "../widgets/widgets_context.h"
// what the host should do after menus_update(). this is just the surfaced action
// from the active screen, post-processed (e.g. BACK is consumed internally by
// popping, so the host never sees it).
typedef menus_action menus_result;
typedef struct menus_manager {
    menus_stack    stack;
    wg_context    *ctx;          // host-owned immediate-mode context

    // host data the screens edit. owned by the host; we hold borrowed pointers.
    menus_settings *settings;    // live settings the host re-reads on APPLY
    menus_settings  edit;        // working copy the settings screen mutates
    keybinds       *binds;       // live keybinds, edited by the rebind screen

    // shared capture state so the keybind screen survives stack churn (it's on
    // the screen's state too, but the manager owns the canonical copy and feeds
    // host keys into it).
    menus_kb_state  kb;

    // panel sizing. the manager picks a centered rect; screens get the inner
    // content area. tweakable but these defaults look right at 1280x720.
    float panel_w;
    float panel_max_h;

    int   in_world;      // 1 if a world is loaded (changes main->pause behaviour)
    int   dirty_settings;// edit differs from live; drives the apply button
} menus_manager;
void menus_init(menus_manager *m, wg_context *ctx,
                menus_settings *settings, keybinds *binds, int in_world);
// open the appropriate root screen (main when no world, pause when in one).
void menus_open_root(menus_manager *m);
// push a screen by id. the manager knows how to construct each concrete screen.
void menus_push(menus_manager *m, menus_screen_id id);
// pop the top screen; closes the whole system if that empties the stack.
void menus_back(menus_manager *m);
int  menus_is_open(const menus_manager *m);
// feed a raw key code from the host (used only while a keybind capture is armed).
// returns 1 if it was consumed by capture.
int  menus_feed_key(menus_manager *m, int keycode);
// run one frame. assumes the host has already pumped wg_begin() with fresh input.
// returns the surfaced action (NONE most frames).
menus_result menus_update(menus_manager *m);
// translate the widget input snapshot's key edges into a single nav intent for
// this frame. exposed mostly for the tests; menus_update calls it internally.
menus_nav_dir menus_translate_nav(const wg_context *ctx);
#endif
