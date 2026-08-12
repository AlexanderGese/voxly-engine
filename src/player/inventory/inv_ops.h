#ifndef PLAYER_INVENTORY_OPS_H
#define PLAYER_INVENTORY_OPS_H

#include "inv_types.h"
#include "inv_grid.h"

// slot-to-slot operations. these are what the ui actually calls when you drag,
// shift-click, or right-click split. they sit on top of inv_stack and operate
// on slot indices into one or two grids. every op returns an inv_result so the
// caller can play the right click/clunk without re-deriving what changed.

// move/merge slot src -> slot dst within the same grid. if same item, merge;
// if different (and dst non-empty), swap; if dst empty, relocate.
inv_result inv_ops_move(inv_grid *g, int src, int dst);

// cross-grid version of the above (e.g. backpack <-> chest).
inv_result inv_ops_move_between(inv_grid *gsrc, int src,
                                inv_grid *gdst, int dst);

// "quick move" a whole slot: shove its contents into the first sensible spot
// in `dst` grid (stack into partials, then empties). returns PARTIAL if some
// got left behind. this is the shift-click behaviour.
inv_result inv_ops_quick_move(inv_grid *gsrc, int src, inv_grid *gdst);

// split: pull half of src into the (empty) dst slot. right-click drag.
inv_result inv_ops_split_half(inv_grid *g, int src, int dst);

// drop a single item from src onto dst (right-click while holding nothing, or
// placing one from a held stack). dst must be empty or same item.
inv_result inv_ops_drop_one(inv_grid *g, int src, int dst);

// distribute the contents of `from` evenly across the listed slots (the
// drag-paint behaviour). n is how many target slots. returns leftover put back
// into `from`. used with the cursor stack.
int inv_ops_spread(inv_stack *from, inv_grid *g, const int *slots, int n);

#endif
