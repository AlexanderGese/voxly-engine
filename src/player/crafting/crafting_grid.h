#ifndef PLAYER_CRAFTING_GRID_H
#define PLAYER_CRAFTING_GRID_H

#include "crafting_types.h"

// the working grid. this is the thing the ui pokes at when the player drags
// items around. matching lives elsewhere; this is just storage + helpers.

void craft_grid_clear(craft_grid *g);

// returns 1 if the cell index is in-bounds and was set.
int  craft_grid_set(craft_grid *g, int x, int y, craft_stack s);
craft_stack craft_grid_get(const craft_grid *g, int x, int y);

// drop one item of `id` onto (x,y). stacks if the cell already holds `id`,
// replaces if empty, otherwise refuses. returns 1 on change.
int  craft_grid_place_one(craft_grid *g, int x, int y, block_id id);

// pull a single item back out of a cell (for ui take-back). returns the id
// removed, or BLOCK_AIR if the cell was empty.
block_id craft_grid_take_one(craft_grid *g, int x, int y);

// count of non-empty cells. cheap, used a lot during matching.
int  craft_grid_occupied(const craft_grid *g);

// compute the tight bounding box of occupied cells. returns 0 if grid empty,
// else 1 and writes box corners (inclusive) + dimensions.
int  craft_grid_bounds(const craft_grid *g, int *x0, int *y0,
                       int *x1, int *y1, int *w, int *h);

// true if every cell is empty.
int  craft_grid_is_empty(const craft_grid *g);

#endif
