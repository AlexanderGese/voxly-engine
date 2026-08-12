#include "inv_query.h"
#include "inv_stack.h"
#include "inv_registry.h"

int inv_query_count(const inv_grid *g, inv_item_id id) {
    if (id == INV_ITEM_NONE) return 0;
    int n = 0;
    for (int i = 0; i < g->count; i++)
        if (g->slots[i].id == id) n += g->slots[i].count;
    return n;
}

int inv_query_slots_with(const inv_grid *g, inv_item_id id) {
    if (id == INV_ITEM_NONE) return 0;
    int n = 0;
    for (int i = 0; i < g->count; i++)
        if (g->slots[i].id == id && g->slots[i].count > 0) n++;
    return n;
}

int inv_query_count_category(const inv_grid *g, inv_category cat) {
    int n = 0;
    for (int i = 0; i < g->count; i++) {
        const inv_stack *s = &g->slots[i];
        if (inv_stack_is_empty(s)) continue;
        if (inv_item_category(s->id) == cat) n += s->count;
    }
    return n;
}

int inv_query_room_for(const inv_grid *g, inv_item_id id) {
    if (id == INV_ITEM_NONE) return 0;
    uint16_t cap = inv_item_max_stack(id);
    int room = 0;
    for (int i = 0; i < g->count; i++) {
        const inv_stack *s = &g->slots[i];
        if (inv_stack_is_empty(s)) {
            room += cap;                       // a whole fresh stack fits here
        } else if (s->id == id && s->count < cap) {
            room += cap - s->count;            // top-off headroom on a partial
        }
    }
    return room;
}

int inv_query_fits(const inv_grid *g, inv_item_id id, int amount) {
    if (amount <= 0) return 1;
    return inv_query_room_for(g, id) >= amount;
}

int inv_query_is_empty(const inv_grid *g) {
    for (int i = 0; i < g->count; i++)
        if (!inv_stack_is_empty(&g->slots[i])) return 0;
    return 1;
}

int inv_query_is_full(const inv_grid *g) {
    // "full" means nothing more would fit anywhere: no empties and every used
    // slot is at its item's max. an unstackable item at 1 counts as full.
    for (int i = 0; i < g->count; i++) {
        const inv_stack *s = &g->slots[i];
        if (inv_stack_is_empty(s)) return 0;
        if (!inv_stack_is_full(s)) return 0;
    }
    return 1;
}

inv_item_id inv_query_dominant(const inv_grid *g, int *out_count) {
    inv_item_id best = INV_ITEM_NONE;
    int best_n = 0;
    // tally each distinct item the dumb way: for every slot, sum its whole-grid
    // total once. cheap enough at these sizes and avoids a temp histogram.
    for (int i = 0; i < g->count; i++) {
        const inv_stack *s = &g->slots[i];
        if (inv_stack_is_empty(s)) continue;
        // skip if we already counted this id earlier in the scan.
        int seen = 0;
        for (int j = 0; j < i; j++)
            if (g->slots[j].id == s->id && !inv_stack_is_empty(&g->slots[j])) {
                seen = 1; break;
            }
        if (seen) continue;
        int total = inv_query_count(g, s->id);
        if (total > best_n) { best_n = total; best = s->id; }
    }
    if (out_count) *out_count = best_n;
    return best;
}

int inv_query_distinct(const inv_grid *g, inv_item_id *out, int cap) {
    int n = 0;
    for (int i = 0; i < g->count && n < cap; i++) {
        const inv_stack *s = &g->slots[i];
        if (inv_stack_is_empty(s)) continue;
        int dup = 0;
        for (int k = 0; k < n; k++)
            if (out[k] == s->id) { dup = 1; break; }
        if (!dup) out[n++] = s->id;
    }
    return n;
}
