#ifndef PLAYER_INVENTORY_QUERY_H
#define PLAYER_INVENTORY_QUERY_H

#include "inv_types.h"
#include "inv_grid.h"

// read-only rollups over a grid. nothing here mutates; it's the stuff the hud,
// the tooltips and the "can i craft this" checks lean on. all of it is O(slots)
// linear scans — the grids are tiny so there's no index to maintain.
//
// these used to be scattered as one-off loops in the ui code; pulling them here
// so the answers are consistent (and so i stop miscounting durability tools as
// stackable in three different places).

// total item count across the grid, ignoring which slots hold it.
int inv_query_count(const inv_grid *g, inv_item_id id);

// how many *slots* hold this item (vs the raw count above).
int inv_query_slots_with(const inv_grid *g, inv_item_id id);

// total items in a whole category. handy for "you have N blocks" summaries.
int inv_query_count_category(const inv_grid *g, inv_category cat);

// how many more of `id` would fit right now (partial headroom + empty slots).
// this is the number you check before a pickup to know if it'll all land.
int inv_query_room_for(const inv_grid *g, inv_item_id id);

// would `amount` of `id` fit without spilling? convenience over room_for.
int inv_query_fits(const inv_grid *g, inv_item_id id, int amount);

// is the grid completely empty / completely full of non-mergeable slots?
int inv_query_is_empty(const inv_grid *g);
int inv_query_is_full(const inv_grid *g);

// the most-stocked item in the grid (highest total count). returns NONE for an
// empty grid; *out_count gets the count if non-NULL.
inv_item_id inv_query_dominant(const inv_grid *g, int *out_count);

// distinct item kinds present. fills `out` (cap entries) and returns the count
// written. order is first-seen slot order, deduped.
int inv_query_distinct(const inv_grid *g, inv_item_id *out, int cap);

#endif
