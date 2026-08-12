#ifndef PLAYER_INVENTORY_PLAYER_H
#define PLAYER_INVENTORY_PLAYER_H

#include "inv_types.h"
#include "inv_grid.h"

// the player's whole inventory: one flat grid that's logically split into a
// main backpack (top INV_MAIN_SLOTS) and a hotbar (bottom INV_HOTBAR_SLOTS),
// plus a "cursor" stack for the item currently held by the mouse during ui
// drags. this is the object the game actually owns one of per player.
//
// slot indexing convention used everywhere:
// 0 .. INV_MAIN_SLOTS-1                  -> main backpack
// INV_MAIN_SLOTS .. INV_TOTAL_SLOTS-1    -> hotbar
// the hotbar view in inv_hotbar.* aliases that tail region.

typedef struct {
    inv_grid  bag;          // INV_MAIN_ROWS+1 rows x INV_COLS, flat
    inv_stack cursor;       // held-by-mouse stack, empty when not dragging
    int       selected;     // 0..INV_HOTBAR_SLOTS-1, which hotbar slot is active
} inv_player;

void inv_player_init(inv_player *p);
void inv_player_free(inv_player *p);

// pick up `amount` of `id` from the world. tries hotbar first (so freshly
// mined blocks land where you can use them), then the backpack. returns the
// leftover that didn't fit.
int  inv_player_pickup(inv_player *p, inv_item_id id, int amount);

// consume from the currently selected hotbar slot (e.g. placing a block).
// returns 1 if at least one was consumed.
int  inv_player_consume_selected(inv_player *p, int amount);

// what's in the selected hotbar slot right now.
inv_item_id inv_player_selected_item(const inv_player *p);

// hotbar selection.
void inv_player_select(inv_player *p, int idx);
void inv_player_scroll(inv_player *p, int delta);

// translate a hotbar-relative index (0..8) into a bag slot index.
int  inv_player_hotbar_slot(int hotbar_idx);

// pick up / put down with the cursor at a bag slot (the ui click handler).
void inv_player_click(inv_player *p, int slot);          // left click
void inv_player_click_right(inv_player *p, int slot);    // right click (split/one)

#endif
