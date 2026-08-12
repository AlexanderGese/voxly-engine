#include "inv_player.h"
#include "inv_stack.h"
#include "inv_ops.h"
#include "inv_registry.h"   // inv_item_max_stack

void inv_player_init(inv_player *p) {
    // one grid, MAIN_ROWS of backpack plus a hotbar row at the bottom.
    inv_grid_init(&p->bag, INV_MAIN_ROWS + 1, INV_COLS);
    p->cursor   = inv_stack_empty();
    p->selected = 0;
}

void inv_player_free(inv_player *p) {
    inv_grid_free(&p->bag);
    p->cursor = inv_stack_empty();
}

int inv_player_hotbar_slot(int hotbar_idx) {
    if (hotbar_idx < 0) hotbar_idx = 0;
    if (hotbar_idx >= INV_HOTBAR_SLOTS) hotbar_idx = INV_HOTBAR_SLOTS - 1;
    return INV_MAIN_SLOTS + hotbar_idx;
}

// add into a sub-range [lo,hi) of the bag, partials then empties. returns
// leftover. shares the same two-pass logic as inv_grid_add but bounded.
static int add_range(inv_grid *g, int lo, int hi, inv_item_id id, int amount) {
    for (int i = lo; i < hi && amount > 0; i++) {
        inv_stack *s = &g->slots[i];
        if (s->id != id || inv_stack_is_full(s)) continue;
        uint16_t room = inv_stack_space(s);
        uint16_t put  = amount < room ? (uint16_t)amount : room;
        s->count = (uint16_t)(s->count + put);
        amount  -= put;
    }
    uint16_t cap = inv_item_max_stack(id);
    for (int i = lo; i < hi && amount > 0; i++) {
        inv_stack *s = &g->slots[i];
        if (!inv_stack_is_empty(s)) continue;
        uint16_t put = amount < cap ? (uint16_t)amount : cap;
        s->id = id; s->count = put;
        amount -= put;
    }
    return amount;
}

int inv_player_pickup(inv_player *p, inv_item_id id, int amount) {
    if (id == INV_ITEM_NONE || amount <= 0) return amount > 0 ? amount : 0;
    // hotbar first so you can immediately use what you grabbed...
    amount = add_range(&p->bag, INV_MAIN_SLOTS, INV_TOTAL_SLOTS, id, amount);
    // ...then the backpack soaks the rest.
    amount = add_range(&p->bag, 0, INV_MAIN_SLOTS, id, amount);
    return amount;
}

inv_item_id inv_player_selected_item(const inv_player *p) {
    int slot = INV_MAIN_SLOTS + p->selected;
    const inv_stack *s = inv_grid_cat(&p->bag, slot);
    return s ? s->id : INV_ITEM_NONE;
}

int inv_player_consume_selected(inv_player *p, int amount) {
    if (amount <= 0) return 0;
    inv_stack *s = inv_grid_at(&p->bag, INV_MAIN_SLOTS + p->selected);
    if (!s || inv_stack_is_empty(s)) return 0;
    int take = s->count < amount ? s->count : amount;
    s->count = (uint16_t)(s->count - take);
    if (s->count == 0) s->id = INV_ITEM_NONE;
    return take > 0;
}

void inv_player_select(inv_player *p, int idx) {
    if (idx < 0 || idx >= INV_HOTBAR_SLOTS) return;
    p->selected = idx;
}

void inv_player_scroll(inv_player *p, int delta) {
    int i = p->selected - delta;
    while (i < 0) i += INV_HOTBAR_SLOTS;
    i %= INV_HOTBAR_SLOTS;
    p->selected = i;
}

void inv_player_click(inv_player *p, int slot) {
    inv_stack *s = inv_grid_at(&p->bag, slot);
    if (!s) return;

    if (inv_stack_is_empty(&p->cursor)) {
        // grab the whole slot into the cursor.
        inv_stack_swap(&p->cursor, s);
        return;
    }
    // holding something: try to put it down.
    if (inv_stack_is_empty(s)) {
        inv_stack_swap(&p->cursor, s);
        return;
    }
    if (s->id == p->cursor.id) {
        inv_stack_merge(s, &p->cursor);     // top off the slot, keep remainder
        return;
    }
    // different item under us: swap cursor and slot.
    inv_stack_swap(&p->cursor, s);
}

void inv_player_click_right(inv_player *p, int slot) {
    inv_stack *s = inv_grid_at(&p->bag, slot);
    if (!s) return;

    if (inv_stack_is_empty(&p->cursor)) {
        // split half of the slot into the cursor.
        inv_stack_split_half(s, &p->cursor);
        return;
    }
    // holding something: drop one onto the slot if it fits.
    inv_stack_take_one(&p->cursor, s);
}
