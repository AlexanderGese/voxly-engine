#ifndef UI_MENUS_NAV_H
#define UI_MENUS_NAV_H
// keyboard/gamepad focus ring for a single screen. mouse hover is handled by the
// widget kernel directly; this layer is the "no mouse" path — up/down move focus
// between rows, left/right nudge the focused control, enter activates it.
//
// each frame a screen registers its navigable items in order via menus_nav_item()
// while it lays them out. the nav remembers the focused index across frames by
// matching on a stable per-item key (the widget id), so inserting/removing rows
// doesn't yank focus to a random control.
//
// this is deliberately tiny: a flat list rebuilt every frame, no tree. menus here
// are shallow enough that a linear ring is all you need.
#include <stdint.h>
#include "menus_types.h"
// max navigable controls on one screen. settings is the biggest and it's ~14.
#define MENUS_NAV_MAX 48
// directional intents the screen translates raw keys into before feeding nav.
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
// drop the remembered focus so the next frame highlights the first item.
void menus_nav_reset(menus_nav *n);
// call at the top of a screen's build, before registering items.
void menus_nav_begin(menus_nav *n);
// register one navigable item in layout order. returns 1 if this item currently
// holds focus, so the screen can draw a focus ring on it without a second pass.
int  menus_nav_item(menus_nav *n, uint32_t key, menus_item slot);
// apply a directional intent. for PREV/NEXT this walks the ring; for the rest it
// just records the intent so the focused control can read it. call after all
void menus_nav_apply(menus_nav *n, menus_nav_dir dir);
int        menus_nav_is_focused(const menus_nav *n, uint32_t key);
menus_item menus_nav_focused_slot(const menus_nav *n);
menus_nav_dir menus_nav_take_intent(menus_nav *n, uint32_t key);
#endif
