#ifndef PLAYER_CRAFTING_TABLE_H
#define PLAYER_CRAFTING_TABLE_H

#include "crafting_types.h"

// a crafting session: the working grid plus the cached result slot. this is
// what the ui holds onto while the crafting screen is open. 2x2 sessions
// (the inventory mini-grid) just refuse to use the bottom row / right column.

typedef struct {
    craft_grid grid;
    craft_stack result;     // cached output preview, recomputed on change
    int         matched_id; // recipe id currently matched, or -1
    int         is_3x3;     // 1 = full table, 0 = 2x2 inventory grid
} craft_session;

void craft_session_init(craft_session *s, int is_3x3);

// recompute the result preview from the current grid. call after any grid
// edit. returns the matched recipe id or -1. respects the 2x2 restriction.
int  craft_session_refresh(craft_session *s);

// place / take one item, then refresh. thin wrappers the ui calls.
int      craft_session_place(craft_session *s, int x, int y, block_id id);
block_id craft_session_take(craft_session *s, int x, int y);

// is there a craftable result right now?
int  craft_session_can_craft(const craft_session *s);

// perform one craft: consume one of every grid ingredient, return the result
// stack via `out`. returns 1 on success, 0 if nothing matched. crafting
// "shift-style" (craft as many as the grid allows) is handled separately.
int  craft_session_craft_one(craft_session *s, craft_stack *out);

// craft repeatedly until the grid can no longer satisfy the recipe, summing
// the yields. returns total items produced; the id is written to *out_id.
int  craft_session_craft_all(craft_session *s, block_id *out_id);

// drop everything in the grid back out (ui close). writes each leftover stack
// into `spill` (caller-sized, up to CRAFT_GRID_CELLS) and returns the count.
int  craft_session_clear(craft_session *s, craft_stack *spill);

#endif
