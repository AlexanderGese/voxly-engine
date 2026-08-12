#include "inv_ops.h"
#include "inv_stack.h"
#include <stddef.h>

// the core "drop stack a onto stack b" rule, shared by same-grid and
// cross-grid moves. a is the source, b is the destination.
static inv_result drop_onto(inv_stack *a, inv_stack *b) {
    if (inv_stack_is_empty(a)) return INV_RESULT_NOOP;

    if (inv_stack_is_empty(b)) {
        inv_stack_swap(a, b);          // relocate whole stack
        return INV_RESULT_MERGED;
    }
    if (a->id == b->id) {
        uint16_t before = a->count;
        uint16_t moved  = inv_stack_merge(b, a);
        if (moved == 0)          return INV_RESULT_NOOP;
        if (a->count == 0)       return INV_RESULT_MERGED;   // all of it folded in
        (void)before;
        return INV_RESULT_PARTIAL;     // dst capped, some left on src
    }
    // different items: trade places.
    inv_stack_swap(a, b);
    return INV_RESULT_SWAPPED;
}

inv_result inv_ops_move(inv_grid *g, int src, int dst) {
    if (src == dst) return INV_RESULT_NOOP;
    inv_stack *a = inv_grid_at(g, src);
    inv_stack *b = inv_grid_at(g, dst);
    if (!a || !b) return INV_RESULT_INVALID;
    return drop_onto(a, b);
}

inv_result inv_ops_move_between(inv_grid *gsrc, int src,
                                inv_grid *gdst, int dst) {
    inv_stack *a = inv_grid_at(gsrc, src);
    inv_stack *b = inv_grid_at(gdst, dst);
    if (!a || !b) return INV_RESULT_INVALID;
    if (a == b)   return INV_RESULT_NOOP;   // same grid, same slot
    return drop_onto(a, b);
}

inv_result inv_ops_quick_move(inv_grid *gsrc, int src, inv_grid *gdst) {
    inv_stack *a = inv_grid_at(gsrc, src);
    if (!a) return INV_RESULT_INVALID;
    if (inv_stack_is_empty(a)) return INV_RESULT_NOOP;

    uint16_t start = a->count;

    // first stack into partials of the same item...
    for (int i = 0; i < gdst->count && a->count > 0; i++) {
        inv_stack *b = &gdst->slots[i];
        if (b->id != a->id || inv_stack_is_full(b)) continue;
        inv_stack_merge(b, a);
    }
    // ...then drop the remainder into empties.
    for (int i = 0; i < gdst->count && a->count > 0; i++) {
        inv_stack *b = &gdst->slots[i];
        if (!inv_stack_is_empty(b)) continue;
        inv_stack_merge(b, a);
    }

    if (a->count == start)  return INV_RESULT_FULL;     // nothing moved at all
    if (a->count == 0)      return INV_RESULT_MERGED;
    return INV_RESULT_PARTIAL;
}

inv_result inv_ops_split_half(inv_grid *g, int src, int dst) {
    if (src == dst) return INV_RESULT_NOOP;
    inv_stack *a = inv_grid_at(g, src);
    inv_stack *b = inv_grid_at(g, dst);
    if (!a || !b) return INV_RESULT_INVALID;
    if (inv_stack_is_empty(a)) return INV_RESULT_NOOP;

    if (inv_stack_is_empty(b)) {
        inv_stack_split_half(a, b);
        return INV_RESULT_MERGED;
    }
    // splitting onto a same-item stack just merges half over.
    if (b->id == a->id && !inv_stack_is_full(b)) {
        uint16_t half = (uint16_t)((a->count + 1) / 2);
        inv_stack_transfer(b, a, half);
        return INV_RESULT_PARTIAL;
    }
    return INV_RESULT_NOOP;
}

inv_result inv_ops_drop_one(inv_grid *g, int src, int dst) {
    if (src == dst) return INV_RESULT_NOOP;
    inv_stack *a = inv_grid_at(g, src);
    inv_stack *b = inv_grid_at(g, dst);
    if (!a || !b) return INV_RESULT_INVALID;
    return inv_stack_take_one(a, b) ? INV_RESULT_MERGED : INV_RESULT_NOOP;
}

int inv_ops_spread(inv_stack *from, inv_grid *g, const int *slots, int n) {
    if (inv_stack_is_empty(from) || n <= 0) return from->count;

    // even share, leftover stays in the cursor. matches the drag-paint feel
    // where you sprinkle one item per slot you sweep over.
    int per = from->count / n;
    if (per < 1) per = 1;

    for (int i = 0; i < n && from->count > 0; i++) {
        inv_stack *b = inv_grid_at(g, slots[i]);
        if (!b) continue;
        if (!inv_stack_is_empty(b) && b->id != from->id) continue;
        inv_stack_transfer(b, from, (uint16_t)per);
    }
    return from->count;
}
