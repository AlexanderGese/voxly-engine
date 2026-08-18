#ifndef UI_MENUS_BUILD_H
#define UI_MENUS_BUILD_H

// internal: the concrete-screen constructors. each menus_make_* fills a
// menus_screen by value (vtbl + freshly-allocated state) which the manager then
// pushes. kept out of menus_manager.h so the public surface stays small — only
// the manager and the screen .c files include this.
//
// the manager passes itself so screens can grab borrowed pointers (settings,
// keybinds) at construction time rather than reaching through globals.

#include "menus_screen.h"

struct menus_manager;

// title screen. buttons: continue (if a world is loaded), new world, settings,
// quit. no per-screen state, so owns_state is 0.
menus_screen menus_make_main(struct menus_manager *m);

// in-world pause. resume / settings / save & quit. shares the settings pointer.
menus_screen menus_make_pause(struct menus_manager *m);

// settings: sliders + toggles + spinners editing the manager's working copy.
// apply/back buttons at the bottom. allocates a small state for scroll offset.
menus_screen menus_make_settings(struct menus_manager *m);

// keybind rebind list with live capture. allocates state holding the capture
// machine + scroll.
menus_screen menus_make_keybinds(struct menus_manager *m);

#endif
