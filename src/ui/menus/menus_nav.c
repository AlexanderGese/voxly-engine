#include "menus_nav.h"

void menus_nav_init(menus_nav *n) {
    n->count     = 0;
    n->focus_key = 0;
    n->focus_idx = -1;
    n->moved     = 0;
}

void menus_nav_reset(menus_nav *n) {
    n->focus_key = 0;
    n->focus_idx = -1;
}

void menus_nav_begin(menus_nav *n) {
    n->count     = 0;
    n->focus_idx = -1;
    n->moved     = 0;
    // pending_intent lives in the high bits of focus state; we stash it in a
    // sentinel slot below. keep it simple: a separate field would be cleaner but
    // we reuse moved-style flags. see take_intent.
    n->_pending  = MENUS_NAV_NONE;
}

int menus_nav_item(menus_nav *n, uint32_t key, menus_item slot) {
    if (n->count >= MENUS_NAV_MAX) {
        // ran out of slots. shouldn't happen with our screens; if it does the
        // tail items just aren't keyboard-navigable, which is a soft failure.
        return 0;
    }
    int idx = n->count;
    n->items[idx].key  = key;
    n->items[idx].slot = slot;
    n->count++;

    // resolve remembered focus the moment we see its key. if focus_key is 0
    // (fresh screen) latch onto the first registered item so something is always
    // selected.
    if (n->focus_key == 0) {
        n->focus_key = key;
        n->focus_idx = idx;
    } else if (key == n->focus_key) {
        n->focus_idx = idx;
    }
    return n->focus_idx == idx;
}

// wrap-around step through the ring. returns the new index.
static int step(int idx, int count, int delta) {
    if (count <= 0) return -1;
    if (idx < 0) return delta > 0 ? 0 : count - 1;
    idx += delta;
    if (idx < 0)       idx = count - 1;
    if (idx >= count)  idx = 0;
    return idx;
}

void menus_nav_apply(menus_nav *n, menus_nav_dir dir) {
    switch (dir) {
    case MENUS_NAV_PREV:
    case MENUS_NAV_NEXT: {
        int delta = (dir == MENUS_NAV_NEXT) ? 1 : -1;
        int idx = step(n->focus_idx, n->count, delta);
        if (idx >= 0 && idx != n->focus_idx) {
            n->focus_idx = idx;
            n->focus_key = n->items[idx].key;
            n->moved = 1;
        }
        break;
    }
    case MENUS_NAV_DEC:
    case MENUS_NAV_INC:
    case MENUS_NAV_ACTIVATE:
        // value intents are routed to the focused control on the *next* build via
        // take_intent. stash it; if nothing is focused it's harmlessly dropped.
        n->_pending = dir;
        break;
    case MENUS_NAV_CANCEL:
    case MENUS_NAV_NONE:
    default:
        break;
    }
}

int menus_nav_is_focused(const menus_nav *n, uint32_t key) {
    return n->focus_idx >= 0 && n->items[n->focus_idx].key == key;
}

menus_item menus_nav_focused_slot(const menus_nav *n) {
    if (n->focus_idx < 0) return MENUS_NO_ITEM;
    return n->items[n->focus_idx].slot;
}

menus_nav_dir menus_nav_take_intent(menus_nav *n, uint32_t key) {
    if (n->_pending == MENUS_NAV_NONE) return MENUS_NAV_NONE;
    if (!menus_nav_is_focused(n, key)) return MENUS_NAV_NONE;
    menus_nav_dir d = n->_pending;
    n->_pending = MENUS_NAV_NONE;   // one-shot
    return d;
}
