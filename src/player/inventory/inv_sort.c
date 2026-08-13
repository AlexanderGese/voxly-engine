#include "inv_sort.h"
#include "inv_stack.h"
#include "inv_registry.h"

int inv_sort_cmp(const inv_stack *a, const inv_stack *b) {
    // empties always sink to the bottom.
    int ea = inv_stack_is_empty(a), eb = inv_stack_is_empty(b);
    if (ea != eb) return ea - eb;        // non-empty (0) before empty (1)
    if (ea && eb) return 0;

    int ca = inv_item_category(a->id);
    int cb = inv_item_category(b->id);
    if (ca != cb) return ca - cb;        // group by category

    if (a->id != b->id) return (int)a->id - (int)b->id;   // then by item id

    return (int)b->count - (int)a->count; // fuller stacks first
}

int inv_sort_compact_range(inv_grid *g, int lo, int hi) {
    if (lo < 0) lo = 0;
    if (hi > g->count) hi = g->count;
    int freed = 0;

    // for each non-full stack, pull from later same-item stacks into it.
    for (int i = lo; i < hi; i++) {
        inv_stack *dst = &g->slots[i];
        if (inv_stack_is_empty(dst) || inv_stack_is_full(dst)) continue;
        for (int j = i + 1; j < hi; j++) {
            inv_stack *src = &g->slots[j];
            if (src->id != dst->id || inv_stack_is_empty(src)) continue;
            int was = src->count;
            inv_stack_merge(dst, src);
            if (was > 0 && src->count == 0) freed++;
            if (inv_stack_is_full(dst)) break;
        }
    }
    return freed;
}

int inv_sort_compact(inv_grid *g) {
    return inv_sort_compact_range(g, 0, g->count);
}

void inv_sort_grid(inv_grid *g, int lo, int hi) {
    if (lo < 0) lo = 0;
    if (hi > g->count) hi = g->count;
    if (hi - lo < 2) return;

    inv_sort_compact_range(g, lo, hi);

    // insertion sort over the window. n is tiny (few dozen slots) so this is
    // plenty fast and keeps it stable without dragging in qsort + a context.
    for (int i = lo + 1; i < hi; i++) {
        inv_stack key = g->slots[i];
        int j = i - 1;
        while (j >= lo && inv_sort_cmp(&g->slots[j], &key) > 0) {
            g->slots[j + 1] = g->slots[j];
            j--;
        }
        g->slots[j + 1] = key;
    }
}
