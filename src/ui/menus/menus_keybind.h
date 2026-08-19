#ifndef UI_MENUS_KEYBIND_H
#define UI_MENUS_KEYBIND_H

// keybind capture. the rebind screen shows a row per action; clicking a row arms
// capture for that action and the very next key the host reports gets written
// into the keybinds struct. escape cancels the capture without changing anything.
//
// the host owns the real `keybinds` (src/input/keybinds.h) and feeds raw key
// codes in via menus_kb_feed_key() while a capture is armed. the menu never reads
// the keyboard itself — same one-way rule as the rest of the subsystem.

#include <stddef.h>   // size_t, offsetof
#include "../../input/keybinds.h"

// how many rebindable actions we expose. order matches the table in the .c so the
// screen can iterate by index.
#define MENUS_KB_ACTIONS 10

// capture state. lives on the keybind screen's per-screen struct.
typedef enum {
    MENUS_KB_IDLE = 0,     // not capturing
    MENUS_KB_ARMED,        // a row is waiting for the next key
} menus_kb_phase;

typedef struct {
    keybinds      *target;     // host-owned, edited in place. NULL = no binding yet
    keybinds       backup;     // snapshot taken on screen-enter, for "reset"
    menus_kb_phase phase;
    int            armed_action;   // index into the action table, -1 when idle
    int            conflict;       // last assigned key collided with another bind
    int            conflict_action;// which action it collided with (-1 none)
} menus_kb_state;

// reflection over the keybinds struct: name + pointer to the int field for each
// rebindable action. built once, returns a pointer into a static table.
typedef struct {
    const char *name;          // human label, e.g. "move forward"
    size_t      field_off;     // offsetof into keybinds
} menus_kb_action;

const menus_kb_action *menus_kb_actions(void);   // [MENUS_KB_ACTIONS]

// get/set an action's bound key code by index, going through the offset table.
int  menus_kb_get(const keybinds *k, int action);
void menus_kb_set(keybinds *k, int action, int keycode);

void menus_kb_init(menus_kb_state *st, keybinds *target);

// arm capture for `action`. a second arm on the same row cancels it (toggle).
void menus_kb_arm(menus_kb_state *st, int action);
void menus_kb_cancel(menus_kb_state *st);

// host pushes the next raw key code here while armed. assigns it, detects
// conflicts (same key already used elsewhere), disarms. returns 1 if a binding
// was written. escape (passed as a sentinel) cancels and returns 0.
#define MENUS_KB_ESCAPE_CODE (-2)
int  menus_kb_feed_key(menus_kb_state *st, int keycode);

// restore the snapshot from enter-time. returns 1 if anything changed.
int  menus_kb_reset(menus_kb_state *st);

// a printable name for a key code, into `buf`. handles letters/digits/space and a
// few named keys; falls back to "key NN".
void menus_kb_key_name(int keycode, char *buf, size_t n);

#endif
