#include "inv_grid.h"
#include "inv_stack.h"
#include "inv_registry.h"
#include <stdlib.h>
#include <string.h>

void inv_grid_init(inv_grid *g, int rows, int cols) {
    if (rows < 0) rows = 0;
    if (cols < 0) cols = 0;
    g->rows  = rows;
    g->cols  = cols;
    g->count = rows * cols;
    g->slots = g->count ? calloc((size_t)g->count, sizeof(inv_stack)) : NULL;
    // calloc gives id=0,count=0 which is already a valid empty slot. nice.
}

void inv_grid_free(inv_grid *g) {
    free(g->slots);
    g->slots = NULL;
    g->rows = g->cols = g->count = 0;
}

void inv_grid_clear(inv_grid *g) {
    if (g->slots) memset(g->slots, 0, (size_t)g->count * sizeof(inv_stack));
}

int inv_grid_in_bounds(const inv_grid *g, int idx) {
    return idx >= 0 && idx < g->count;
}

inv_stack *inv_grid_at(inv_grid *g, int idx) {
    return inv_grid_in_bounds(g, idx) ? &g->slots[idx] : NULL;
}

const inv_stack *inv_grid_cat(const inv_grid *g, int idx) {
    return inv_grid_in_bounds(g, idx) ? &g->slots[idx] : NULL;
}

inv_stack *inv_grid_rc(inv_grid *g, int row, int col) {
    if (row < 0 || row >= g->rows || col < 0 || col >= g->cols) return NULL;
    return &g->slots[row * g->cols + col];
}

int inv_grid_empty_slots(const inv_grid *g) {
    int n = 0;
    for (int i = 0; i < g->count; i++)
        if (inv_stack_is_empty(&g->slots[i])) n++;
    return n;
}

int inv_grid_used_slots(const inv_grid *g) {
    return g->count - inv_grid_empty_slots(g);
}

int inv_grid_total(const inv_grid *g, inv_item_id id) {
    int n = 0;
    for (int i = 0; i < g->count; i++)
        if (g->slots[i].id == id && id != INV_ITEM_NONE)
            n += g->slots[i].count;
    return n;
}

int inv_grid_has(const inv_grid *g, inv_item_id id, int amount) {
    return inv_grid_total(g, id) >= amount;
}

int inv_grid_first_empty(const inv_grid *g) {
    for (int i = 0; i < g->count; i++)
        if (inv_stack_is_empty(&g->slots[i])) return i;
    return -1;
}

int inv_grid_find(const inv_grid *g, inv_item_id id) {
    if (id == INV_ITEM_NONE) return -1;
    for (int i = 0; i < g->count; i++)
        if (g->slots[i].id == id && g->slots[i].count > 0) return i;
    return -1;
}

int inv_grid_find_mergeable(const inv_grid *g, inv_item_id id) {
    if (id == INV_ITEM_NONE) return -1;
    for (int i = 0; i < g->count; i++) {
        const inv_stack *s = &g->slots[i];
        if (s->id == id && !inv_stack_is_full(s)) return i;
    }
    return -1;
}

int inv_grid_add(inv_grid *g, inv_item_id id, int amount) {
    if (id == INV_ITEM_NONE || amount <= 0) return amount > 0 ? amount : 0;

    // pass 1: top off existing partial stacks. cheapest place for the items
    // to land and it keeps the bag tidy.
    for (int i = 0; i < g->count && amount > 0; i++) {
        inv_stack *s = &g->slots[i];
        if (s->id != id || inv_stack_is_full(s)) continue;
        uint16_t room = inv_stack_space(s);
        uint16_t put  = amount < room ? (uint16_t)amount : room;
        s->count = (uint16_t)(s->count + put);
        amount  -= put;
    }

    // pass 2: spill the rest into empty slots, one fresh stack at a time.
    uint16_t cap = inv_item_max_stack(id);
    for (int i = 0; i < g->count && amount > 0; i++) {
        inv_stack *s = &g->slots[i];
        if (!inv_stack_is_empty(s)) continue;
        uint16_t put = amount < cap ? (uint16_t)amount : cap;
        s->id    = id;
        s->count = put;
        amount  -= put;
    }

    return amount;   // whatever's left over couldn't fit
}

int inv_grid_remove(inv_grid *g, inv_item_id id, int amount) {
    if (id == INV_ITEM_NONE || amount <= 0) return 0;
    int removed = 0;
    // pull from the back so the hotbar (usually the front rows) drains last.
    for (int i = g->count - 1; i >= 0 && amount > 0; i--) {
        inv_stack *s = &g->slots[i];
        if (s->id != id || s->count == 0) continue;
        int take = s->count < amount ? s->count : amount;
        s->count = (uint16_t)(s->count - take);
        if (s->count == 0) s->id = INV_ITEM_NONE;
        amount  -= take;
        removed += take;
    }
    return removed;
}
