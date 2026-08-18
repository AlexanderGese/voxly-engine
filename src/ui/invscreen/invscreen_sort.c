#include "invscreen_sort.h"

#include <string.h>

// grab the grid region bounds once. everything here works on absolute indices
// in [base, base+cnt).
static void grid_span(int *base, int *cnt) {
    *base = invscreen_model_region_base(INVSCR_REGION_GRID);
    *cnt  = invscreen_model_region_count(INVSCR_REGION_GRID);
}

int invscreen_sort_compact(invscreen_model *m) {
    int base, cnt;
    grid_span(&base, &cnt);

    int freed = 0;
    // for each cell, pour every later same-id cell into it until it's full. when
    // a later cell empties out it counts as freed. classic O(n^2) but n is 27.
    for (int i = 0; i < cnt; i++) {
        invscreen_slot *dst = invscreen_model_at(m, base + i);
        if (invscreen_slot_is_empty(dst)) continue;
        if (dst->count >= INVSCR_STACK_MAX) continue;

        for (int j = i + 1; j < cnt; j++) {
            invscreen_slot *src = invscreen_model_at(m, base + j);
            if (invscreen_slot_is_empty(src)) continue;
            if (src->block != dst->block) continue;

            invscreen_slot_transfer(dst, src, src->count);
            if (invscreen_slot_is_empty(src)) freed++;
            if (dst->count >= INVSCR_STACK_MAX) break;
        }
    }
    return freed;
}

// ordering key for two non-empty slots. lower comes first: by block id ascending,
// then by count descending so the big stacks lead. empties are handled by the
// caller (they always sort last).
static int slot_before(const invscreen_slot *a, const invscreen_slot *b) {
    if (a->block != b->block) return a->block < b->block;
    return a->count > b->count;
}

int invscreen_sort_grid(invscreen_model *m) {
    int base, cnt;
    grid_span(&base, &cnt);

    // snapshot the pre-sort state so we can report whether anything moved.
    invscreen_slot before[INVSCR_GRID_SLOTS];
    for (int i = 0; i < cnt; i++)
        before[i] = *invscreen_model_at(m, base + i);

    invscreen_sort_compact(m);

    // simple insertion sort over the grid cells. empties bubble to the end
    // because slot_before only ever returns true for non-empty `a`, and we treat
    // empty `a` as "not before anything".
    for (int i = 1; i < cnt; i++) {
        invscreen_slot key = *invscreen_model_at(m, base + i);
        if (invscreen_slot_is_empty(&key)) continue;

        int j = i - 1;
        while (j >= 0) {
            invscreen_slot *cur = invscreen_model_at(m, base + j);
            // shift `cur` right if it's empty (key should precede it) or if key
            // sorts before it.
            int shift = invscreen_slot_is_empty(cur) || slot_before(&key, cur);
            if (!shift) break;
            *invscreen_model_at(m, base + j + 1) = *cur;
            j--;
        }
        *invscreen_model_at(m, base + j + 1) = key;
    }

    // did the contents actually change order?
    for (int i = 0; i < cnt; i++) {
        const invscreen_slot *now = invscreen_model_at(m, base + i);
        if (now->block != before[i].block || now->count != before[i].count)
            return 1;
    }
    return 0;
}
