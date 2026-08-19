#ifndef UI_MENUS_NAV_H
#define UI_MENUS_NAV_H
#include <stdint.h>
#include "menus_types.h"
#define MENUS_NAV_MAX 48
typedef enum {
    MENUS_NAV_NONE = 0,
    MENUS_NAV_PREV,      // up / shift-tab
    MENUS_NAV_NEXT,      // down / tab
    MENUS_NAV_DEC,       // left  — nudge value down
    MENUS_NAV_INC,       // right — nudge value up
    MENUS_NAV_ACTIVATE,  // enter / space
    MENUS_NAV_CANCEL,    // escape — usually maps to BACK
} menus_nav_dir;
typedef struct {
    uint32_t key;        // stable id (widget id) for cross-frame matching
    menus_item slot;     // the screen's own item index, opaque to nav
} menus_nav_entry;
typedef struct {
    menus_nav_entry items[MENUS_NAV_MAX];
    int   count;         // items registered this frame

    uint32_t focus_key;  // remembered focus, matched next frame
    int   focus_idx;     // resolved index into items[], -1 if focus_key not found

    // edge: did focus move this frame. screens use it to scroll the focused row
    // into view.
    int   moved;

    // stashed value intent (dec/inc/activate) waiting to be drained by the
    // focused control. underscore-prefixed because it's nav-internal plumbing,
    // not something a screen should poke.
    menus_nav_dir _pending;
} menus_nav;
#endif
