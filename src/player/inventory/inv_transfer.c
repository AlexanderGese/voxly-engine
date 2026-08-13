#include "inv_transfer.h"
#include "inv_stack.h"
#include "inv_registry.h"
#include <string.h>

// pour `src` slot into a chosen dst slot, gated by the filter array. returns
// how many moved. small wrapper so the two-pass logic below stays readable.
static uint16_t pour(inv_stack *a, inv_grid *dst, int di,
                     const inv_filter *df) {
    if (!inv_filter_slot_accepts(df, dst, di, a->id)) return 0;
    return inv_stack_merge(&dst->slots[di], a);
}

int inv_transfer_drain_slot(inv_grid *src, int src_idx, inv_grid *dst,
                            const inv_filter *dst_filters) {
    inv_stack *a = inv_grid_at(src, src_idx);
    if (!a || inv_stack_is_empty(a)) return 0;

    uint16_t start = a->count;

    // pass 1: top off partials of the same item, cheapest landing spot.
    for (int i = 0; i < dst->count && a->count > 0; i++) {
        inv_stack *b = &dst->slots[i];
        if (b->id != a->id || inv_stack_is_full(b)) continue;
        pour(a, dst, i, dst_filters);
    }
    // pass 2: spill the rest into empty slots the filter allows.
    for (int i = 0; i < dst->count && a->count > 0; i++) {
        if (!inv_stack_is_empty(&dst->slots[i])) continue;
        pour(a, dst, i, dst_filters);
    }
    return (int)(start - a->count);
}

int inv_transfer_item(inv_grid *src, inv_grid *dst,
                      inv_item_id id, int amount,
                      const inv_filter *dst_filters) {
    if (id == INV_ITEM_NONE || amount <= 0) return 0;
    int moved = 0;
    // walk source slots holding this item, draining each until our budget runs
    // out. drain back-to-front so the hotbar (front rows) empties last, mirroring
    // inv_grid_remove's bias.
    for (int i = src->count - 1; i >= 0 && moved < amount; i--) {
        inv_stack *a = &src->slots[i];
        if (a->id != id || inv_stack_is_empty(a)) continue;

        // temporarily cap how much of this slot we're willing to move by
        // splitting the surplus aside, draining, then re-folding the remainder.
        int budget = amount - moved;
        if (a->count <= budget) {
            moved += inv_transfer_drain_slot(src, i, dst, dst_filters);
        } else {
            inv_stack hold = { a->id, (uint16_t)(a->count - budget) };
            a->count = (uint16_t)budget;
            int m = inv_transfer_drain_slot(src, i, dst, dst_filters);
            // whatever didn't move plus the held-back surplus stays in the slot.
            uint16_t leftover = (uint16_t)((budget - m) + hold.count);
            a->id    = leftover ? id : INV_ITEM_NONE;
            a->count = leftover;
            moved   += m;
        }
    }
    return moved;
}

int inv_transfer_all(inv_grid *src, inv_grid *dst,
                     const inv_filter *dst_filters) {
    int moved = 0;
    for (int i = 0; i < src->count; i++)
        moved += inv_transfer_drain_slot(src, i, dst, dst_filters);
    return moved;
}

int inv_transfer_category(inv_grid *src, inv_grid *dst, inv_category cat,
                          const inv_filter *dst_filters) {
    int moved = 0;
    for (int i = 0; i < src->count; i++) {
        inv_stack *a = &src->slots[i];
        if (inv_stack_is_empty(a)) continue;
        if (inv_item_category(a->id) != cat) continue;
        moved += inv_transfer_drain_slot(src, i, dst, dst_filters);
    }
    return moved;
}

int inv_transfer_restock(inv_grid *src, inv_grid *dst,
                         const inv_filter *dst_filters) {
    int moved = 0;
    // for each item dst already holds, pull enough from src to fill its partials.
    // we don't open new dst slots: restock tops up, it doesn't expand.
    for (int di = 0; di < dst->count; di++) {
        inv_stack *b = &dst->slots[di];
        if (inv_stack_is_empty(b) || inv_stack_is_full(b)) continue;
        if (!inv_filter_slot_accepts(dst_filters, dst, di, b->id)) continue;

        for (int si = src->count - 1; si >= 0 && !inv_stack_is_full(b); si--) {
            inv_stack *a = &src->slots[si];
            if (a->id != b->id || inv_stack_is_empty(a)) continue;
            moved += inv_stack_merge(b, a);
        }
    }
    return moved;
}
