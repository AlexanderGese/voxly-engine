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
for (int i = 0;
i < g->count;
i++)
        if (inv_stack_is_empty(&g->slots[i])) n++;
return n;
}

int inv_grid_used_slots(const inv_grid *g) {
    return g->count - inv_grid_empty_slots(g);
}

int inv_grid_total(const inv_grid *g, inv_item_id id) {
    int n = 0;
for (int i = 0;
i < g->count;
i++)
        if (g->slots[i].id == id && id != INV_ITEM_NONE)
            n += g->slots[i].count;
return n;
}

int inv_grid_has(const inv_grid *g, inv_item_id id, int amount) {
    return inv_grid_total(g, id) >= amount;
}

int inv_grid_first_empty(const inv_grid *g) {
    for (int i = 0;
i < g->count;
i++)
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
for (int i = 0;
i < g->count;
i++) {
        const inv_stack *s = &g->slots[i];
        if (s->id == id && !inv_stack_is_full(s)) return i;
    }
    return -1;
int removed = 0;
for (int i = g->count - 1;
i >= 0 && amount > 0;
}
