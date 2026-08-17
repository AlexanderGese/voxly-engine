#include "invscreen_drag.h"
#include <string.h>
void invscreen_drag_init(invscreen_drag *d) {
    memset(d, 0, sizeof *d);
}

// is this index the craft-out cell? you can take from it but never drop into it,
// and grabbing from it has to consume the recipe (handled by the controller via
// the result code), so the drag layer treats it as take-only here.
static int is_craft_out(int slot) {
    return slot == invscreen_model_region_base(INVSCR_REGION_CRAFT_OUT);
}

// pick the whole stack under `slot` onto the cursor, assuming the cursor is
// empty. straight transfer of the full count.
static int grab_all(invscreen_model *m, int slot) {
    invscreen_slot *held = invscreen_model_held(m);
    invscreen_slot *s    = invscreen_model_at(m, slot);
    if (!s || invscreen_slot_is_empty(s)) return 0;
    invscreen_slot_transfer(held, s, s->count);
    return 1;
}

int invscreen_drag_left(invscreen_drag *d, invscreen_model *m, int slot, int shift) {
    invscreen_slot *held = invscreen_model_held(m);
// click outside any slot while holding -> drop the held stack back to the
// world. we don't model the world here; the controller spawns a dropped_item
// and zeroes the held slot, so just report PLACED and leave held alone.
if (slot == INVSCR_NO_SLOT) {
        return invscreen_model_holding(m) ? INVSCR_DRAG_PLACED : INVSCR_DRAG_NONE;
    }

    invscreen_slot *s = invscreen_model_at(m, slot);
if (!s) return INVSCR_DRAG_NONE;
if (shift) {
        return invscreen_slot_is_empty(s) ? INVSCR_DRAG_NONE : INVSCR_DRAG_PLACED;
    }

    // taking from the craft output is special: only allowed when the cursor is
    // empty or holds the same id with room. report GRABBED so the controller can
    // consume the recipe.
    if (is_craft_out(slot)) {
        if (invscreen_slot_is_empty(s)) return INVSCR_DRAG_NONE;
if (invscreen_slot_is_empty(held)) { grab_all(m, slot); return INVSCR_DRAG_GRABBED; }
        if (held->block == s->block &&
            invscreen_slot_room(held) >= s->count) {
            invscreen_slot_transfer(held, s, s->count);
return INVSCR_DRAG_GRABBED;
}
        return INVSCR_DRAG_NONE;
}

    if (invscreen_slot_is_empty(held)) {
        // empty hand: pick the whole stack up. begin a potential paint sweep so a
        // subsequent drag spreads it. (we only paint if there's >1 in hand.)
        if (invscreen_slot_is_empty(s)) return INVSCR_DRAG_NONE;
        grab_all(m, slot);
        d->painting    = held->count > 1;
        d->paint_count = held->count;
        memset(d->paint_visited, 0, sizeof d->paint_visited);
        d->paint_n = 0;
        return INVSCR_DRAG_GRABBED;
    }

    // holding something. drop / merge / swap into the target.
    if (invscreen_slot_is_empty(s)) {
        invscreen_slot_transfer(s, held, held->count);
return INVSCR_DRAG_PLACED;
}
    if (s->block == held->block) {
        int moved = invscreen_slot_transfer(s, held, held->count);
        return moved ? INVSCR_DRAG_PLACED : INVSCR_DRAG_NONE;
    }
    // different ids: swap stack and cursor. only legal if the held stack fits a
    // single slot, which it always does (capped at STACK_MAX).
    invscreen_slot_swap(s, held);
return INVSCR_DRAG_PLACED;
}

int invscreen_drag_right(invscreen_drag *d, invscreen_model *m, int slot) {
    (void)d;
    if (slot == INVSCR_NO_SLOT) return INVSCR_DRAG_NONE;

    invscreen_slot *held = invscreen_model_held(m);
    invscreen_slot *s    = invscreen_model_at(m, slot);
    if (!s) return INVSCR_DRAG_NONE;

    // can't split the craft output — it's all-or-nothing.
    if (is_craft_out(slot)) return INVSCR_DRAG_NONE;

    if (invscreen_slot_is_empty(held)) {
        // pick up half (round up), leaving the rest behind.
        if (invscreen_slot_is_empty(s)) return INVSCR_DRAG_NONE;
        int half = (s->count + 1) / 2;
        invscreen_slot_transfer(held, s, half);
        return INVSCR_DRAG_SPLIT;
    }

    // holding: drop a single item onto the target if it can stack there.
    if (invscreen_slot_is_empty(s) ||
        (s->block == held->block && s->count < INVSCR_STACK_MAX)) {
        invscreen_slot one = invscreen_slot_make(held->block, 1);
        invscreen_slot_transfer(s, &one, 1);
        held->count--;
        if (held->count <= 0) *held = invscreen_slot_empty();
        return INVSCR_DRAG_SPLIT;
    }
    return INVSCR_DRAG_NONE;
}

int invscreen_drag_paint(invscreen_drag *d, invscreen_model *m, int slot) {
    if (!d->painting || slot == INVSCR_NO_SLOT) return INVSCR_DRAG_NONE;
if (slot < 0 || slot >= INVSCR_SLOT_TOTAL) return INVSCR_DRAG_NONE;
if (d->paint_visited[slot]) return INVSCR_DRAG_NONE;
if (is_craft_out(slot)) return INVSCR_DRAG_NONE;
invscreen_slot *held = invscreen_model_held(m);
invscreen_slot *s    = invscreen_model_at(m, slot);
if (!s || invscreen_slot_is_empty(held)) return INVSCR_DRAG_NONE;
if (!invscreen_slot_is_empty(s) &&
        (s->block != held->block || s->count >= INVSCR_STACK_MAX))
        return INVSCR_DRAG_NONE;
invscreen_slot one = invscreen_slot_make(held->block, 1);
if (invscreen_slot_transfer(s, &one, 1)) {
        held->count--;
        if (held->count <= 0) *held = invscreen_slot_empty();
        d->paint_visited[slot] = 1;
        d->paint_n++;
        return INVSCR_DRAG_PAINTED;
    }
    return INVSCR_DRAG_NONE;
}

void invscreen_drag_release(invscreen_drag *d) {
    d->painting    = 0;
    d->paint_count = 0;
    d->paint_n     = 0;
    memset(d->paint_visited, 0, sizeof d->paint_visited);
}
