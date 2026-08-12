#ifndef PLAYER_INVENTORY_LOADOUT_H
#define PLAYER_INVENTORY_LOADOUT_H

#include "inv_types.h"
#include "inv_player.h"

// hotbar loadouts: named snapshots of the hotbar *arrangement* (which item id
// sits in which of the 9 slots), so a player can flip between a "mining" bar and
// a "building" bar. we snapshot item ids and the selected index only — not the
// counts, because the bar is restocked from the bag on apply, not teleported.
//
// this is a creature-comfort feature layered on top of inv_player; it never owns
// items, just remembers a layout and asks inv_transfer to refill it.

#define INV_LOADOUT_MAX     4    // how many bars you can stash
#define INV_LOADOUT_NAMELEN 16

typedef struct {
    char        name[INV_LOADOUT_NAMELEN];
    inv_item_id slot[INV_HOTBAR_SLOTS];   // desired item per hotbar slot
    int         selected;                 // remembered selection
    int         used;                     // 0 = empty bank entry
} inv_loadout;

typedef struct {
    inv_loadout bank[INV_LOADOUT_MAX];
} inv_loadout_bank;

void inv_loadout_bank_init(inv_loadout_bank *bank);

// capture the player's current hotbar layout into bank slot `idx` under `name`.
// returns 0 on success, -1 on a bad index.
int inv_loadout_save(inv_loadout_bank *bank, int idx, const char *name,
                     const inv_player *p);

// apply a stored loadout to the player: for each desired hotbar item, make sure
// that item sits in that slot, pulling from the rest of the bag if needed. items
// already in the right spot stay put. returns the number of slots it managed to
// satisfy (so the ui can warn "couldn't find your pickaxe").
int inv_loadout_apply(const inv_loadout_bank *bank, int idx, inv_player *p);

// drop a stored loadout. returns 0 / -1.
int inv_loadout_clear(inv_loadout_bank *bank, int idx);

// borrow a stored entry for the ui to draw its label, NULL if empty/oob.
const inv_loadout *inv_loadout_peek(const inv_loadout_bank *bank, int idx);

#endif
