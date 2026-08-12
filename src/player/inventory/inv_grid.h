#ifndef PLAYER_INVENTORY_GRID_H
#define PLAYER_INVENTORY_GRID_H

#include "inv_types.h"

// a bag of slots laid out as rows x cols. owns its slot array (malloc'd).
// this is the workhorse container: hotbar, main backpack, and chests are all
// just grids of different shapes. higher level move/swap logic lives in
// inv_ops; this file is pure bookkeeping over the flat slot array.

void inv_grid_init(inv_grid *g, int rows, int cols);
void inv_grid_free(inv_grid *g);
void inv_grid_clear(inv_grid *g);              // empty every slot, keep memory

// flat slot access. returns NULL on out-of-range so loops can bail. the (r,c)
// variant folds into the flat index.
inv_stack *inv_grid_at(inv_grid *g, int idx);
inv_stack *inv_grid_rc(inv_grid *g, int row, int col);
const inv_stack *inv_grid_cat(const inv_grid *g, int idx);

int  inv_grid_in_bounds(const inv_grid *g, int idx);

// queries
int  inv_grid_empty_slots(const inv_grid *g);
int  inv_grid_used_slots(const inv_grid *g);
int  inv_grid_total(const inv_grid *g, inv_item_id id);   // total count of id
int  inv_grid_has(const inv_grid *g, inv_item_id id, int amount);

// find helpers. return slot index or -1.
int  inv_grid_first_empty(const inv_grid *g);
int  inv_grid_find(const inv_grid *g, inv_item_id id);            // any slot
int  inv_grid_find_mergeable(const inv_grid *g, inv_item_id id);  // partial stack

// add `amount` of `id`, stacking into partials first then filling empties.
// returns the leftover that didn't fit (0 == all placed).
int  inv_grid_add(inv_grid *g, inv_item_id id, int amount);

// remove up to `amount` of `id` across slots. returns how many were actually
// removed (<= amount).
int  inv_grid_remove(inv_grid *g, inv_item_id id, int amount);

#endif
