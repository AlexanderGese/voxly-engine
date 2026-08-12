#ifndef PLAYER_INVENTORY_HOTBAR_H
#define PLAYER_INVENTORY_HOTBAR_H

#include "inv_types.h"
#include "inv_player.h"

// a thin read-mostly view over the hotbar row of an inv_player. the hotbar is
// just the last INV_HOTBAR_SLOTS slots of the player bag; this wraps the index
// math and adds the bits the hud cares about (selected stack, place block).
// no storage of its own, it borrows the player.

typedef struct {
    inv_player *p;        // borrowed, not owned
} inv_hotbar;

void inv_hotbar_bind(inv_hotbar *h, inv_player *p);

// read slot 0..INV_HOTBAR_SLOTS-1. returns a borrowed pointer, NULL if oob.
inv_stack *inv_hotbar_slot(inv_hotbar *h, int idx);
const inv_stack *inv_hotbar_cslot(const inv_hotbar *h, int idx);

// the currently selected stack and its place block (BLOCK_AIR if the item is
// not placeable or the slot is empty).
inv_stack *inv_hotbar_selected(inv_hotbar *h);
block_id   inv_hotbar_place_block(const inv_hotbar *h);

// is the selected slot usable for placing right now?
int inv_hotbar_can_place(const inv_hotbar *h);

// cycle: move the selection by delta, wrapping. mirrors the player scroll but
// lives here so hud code doesn't reach through to the player struct.
void inv_hotbar_cycle(inv_hotbar *h, int delta);
int  inv_hotbar_selected_index(const inv_hotbar *h);

#endif
