#ifndef UI_MENUS_NAV_H
#define UI_MENUS_NAV_H
// keyboard/gamepad focus ring for a single screen. mouse hover is handled by the
// widget kernel directly; this layer is the "no mouse" path — up/down move focus
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
void menus_nav_init(menus_nav *n);
void menus_nav_reset(menus_nav *n);
void menus_nav_begin(menus_nav *n);
int  menus_nav_item(menus_nav *n, uint32_t key, menus_item slot);
void menus_nav_apply(menus_nav *n, menus_nav_dir dir);
int        menus_nav_is_focused(const menus_nav *n, uint32_t key);
menus_item menus_nav_focused_slot(const menus_nav *n);
menus_nav_dir menus_nav_take_intent(menus_nav *n, uint32_t key);
#endif
