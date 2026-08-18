#include "menus_keybind.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

// the rebindable action table. order is load-bearing: the screen iterates 0..N
// and menus_kb_get/set index into here. we only expose the movement + util binds;
// the inv_1..9 hotbar keys aren't rebindable in this menu (they're positional).
static const menus_kb_action g_actions[MENUS_KB_ACTIONS] = {
    { "move forward",  offsetof(keybinds, forward) },
    { "move back",     offsetof(keybinds, back)    },
    { "strafe left",   offsetof(keybinds, left)    },
    { "strafe right",  offsetof(keybinds, right)   },
    { "jump",          offsetof(keybinds, jump)    },
    { "sprint",        offsetof(keybinds, sprint)  },
    { "sneak",         offsetof(keybinds, sneak)   },
    { "toggle fly",    offsetof(keybinds, fly)     },
    { "debug overlay", offsetof(keybinds, debug)   },
    { "open menu",     offsetof(keybinds, menu)    },
};

const menus_kb_action *menus_kb_actions(void) {
    return g_actions;
}

// field access through the offset table. keep the casts in one place so a wrong
// offset blows up here and not at twelve call sites.
int menus_kb_get(const keybinds *k, int action) {
    if (action < 0 || action >= MENUS_KB_ACTIONS) return 0;
    const char *base = (const char *)k;
    return *(const int *)(base + g_actions[action].field_off);
}

void menus_kb_set(keybinds *k, int action, int keycode) {
    if (action < 0 || action >= MENUS_KB_ACTIONS) return;
    char *base = (char *)k;
    *(int *)(base + g_actions[action].field_off) = keycode;
}

void menus_kb_init(menus_kb_state *st, keybinds *target) {
    st->target          = target;
    st->phase           = MENUS_KB_IDLE;
    st->armed_action    = -1;
    st->conflict        = 0;
    st->conflict_action = -1;
    if (target) st->backup = *target;       // value copy snapshot
    else        memset(&st->backup, 0, sizeof st->backup);
}

void menus_kb_arm(menus_kb_state *st, int action) {
    if (action < 0 || action >= MENUS_KB_ACTIONS) return;
    // toggling the same row off feels right — click once to arm, again to bail.
    if (st->phase == MENUS_KB_ARMED && st->armed_action == action) {
        menus_kb_cancel(st);
        return;
    }
    st->phase        = MENUS_KB_ARMED;
    st->armed_action = action;
    st->conflict     = 0;
    st->conflict_action = -1;
}

void menus_kb_cancel(menus_kb_state *st) {
    st->phase        = MENUS_KB_IDLE;
    st->armed_action = -1;
}

// scan the other actions for a key collision. returns the colliding action index
// or -1. we don't auto-resolve; we just flag it so the screen can warn and the
// user decides.
static int find_conflict(const keybinds *k, int keycode, int skip) {
    for (int i = 0; i < MENUS_KB_ACTIONS; i++) {
        if (i == skip) continue;
        if (menus_kb_get(k, i) == keycode) return i;
    }
    return -1;
}

int menus_kb_feed_key(menus_kb_state *st, int keycode) {
    if (st->phase != MENUS_KB_ARMED || !st->target) return 0;

    if (keycode == MENUS_KB_ESCAPE_CODE) {
        // bail without touching the binding.
        menus_kb_cancel(st);
        return 0;
    }

    int action = st->armed_action;
    int clash  = find_conflict(st->target, keycode, action);

    menus_kb_set(st->target, action, keycode);
    st->conflict        = (clash >= 0);
    st->conflict_action = clash;

    menus_kb_cancel(st);
    return 1;
}

int menus_kb_reset(menus_kb_state *st) {
    if (!st->target) return 0;
    int changed = memcmp(st->target, &st->backup, sizeof st->backup) != 0;
    *st->target = st->backup;
    st->conflict        = 0;
    st->conflict_action = -1;
    menus_kb_cancel(st);
    return changed;
}

void menus_kb_key_name(int keycode, char *buf, size_t n) {
    if (n == 0) return;

    // printable ascii letters/digits, uppercased for letters so "W" reads like
    // the key cap. the host hands us ascii-ish codes; anything weird falls to the
    // numeric form.
    if (keycode >= 'a' && keycode <= 'z') {
        buf[0] = (char)(keycode - 'a' + 'A');
        if (n > 1) buf[1] = '\0';
        return;
    }
    if ((keycode >= 'A' && keycode <= 'Z') ||
        (keycode >= '0' && keycode <= '9')) {
        buf[0] = (char)keycode;
        if (n > 1) buf[1] = '\0';
        return;
    }

    const char *named = NULL;
    switch (keycode) {
    case ' ':  named = "space";  break;
    case '\t': named = "tab";    break;
    case '\r': named = "enter";  break;
    case 0:    named = "unset";  break;
    case MENUS_KB_ESCAPE_CODE: named = "esc"; break;
    default: break;
    }
    if (named) {
        snprintf(buf, n, "%s", named);
    } else {
        snprintf(buf, n, "key %d", keycode);
    }
}
