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
return INVSCR_DRAG_GRABBED;
}
        return INVSCR_DRAG_NONE;
return INVSCR_DRAG_PLACED;
return INVSCR_DRAG_PLACED;
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
