#ifndef PLAYER_INVENTORY_SORT_H
#define PLAYER_INVENTORY_SORT_H

#include "inv_types.h"
#include "inv_grid.h"

// tidy-up passes for a grid. compaction merges scattered partial stacks of the
// same item; sort reorders slots by category then item id then count. both are
// in-place and stable enough that calling them repeatedly is a no-op.
//
// the sort restricts itself to a [lo,hi) slot window so you can sort just the
// backpack and leave the player's hand-arranged hotbar alone.

// merge partial stacks of the same item together, leaving empties trailing.
// returns the number of slots freed up.
int inv_sort_compact(inv_grid *g);

// same but bounded to [lo,hi).
int inv_sort_compact_range(inv_grid *g, int lo, int hi);

// full tidy: compact then sort the [lo,hi) window into a stable order.
void inv_sort_grid(inv_grid *g, int lo, int hi);

// comparison used by the sort, exposed for tests / custom callers. negative if
// a should come before b.
int inv_sort_cmp(const inv_stack *a, const inv_stack *b);

#endif
