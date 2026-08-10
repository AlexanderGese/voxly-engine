#ifndef PLAYER_ENCHANT_TABLE_H
#define PLAYER_ENCHANT_TABLE_H

#include "enchant_types.h"
#include "../../math/rng.h"
#include "../../world/world.h"

// the enchanting table ui-state machine. when a player opens a table we snap-
// shot the three offered slots from the current power and a per-table seed.
// the offers stay frozen until the item changes or an enchant is taken, which
// is the whole point of the seed: you can't reroll by reopening.

typedef enum {
    ENCHANT_SLOT_EMPTY = 0,   // not enough power for this slot
    ENCHANT_SLOT_OFFER,       // showing an offer, affordable or not
    ENCHANT_SLOT_TAKEN,       // already consumed this open
} enchant_slot_state;

typedef struct {
    enchant_slot_state state;
    int                cost_levels;   // xp levels needed to take it
    int                preview_id;    // the one enchant shown as a teaser
    int                preview_level;
    enchant_set        result;        // full hidden result, revealed on take
} enchant_slot;

typedef struct {
    uint64_t      seed;        // stable per table instance
    int           shelves;     // cached power at open time
    enchant_cat   item_cat;    // category of the item on the table
    int           item_ench;   // enchantability of that item's material
    int           has_item;
    enchant_slot  slots[ENCHANT_TABLE_SLOTS];
} enchant_table;

// initialize a table with a fresh seed (mix a world seed and the table's
// world position so two tables differ).
void enchant_table_init(enchant_table *t, unsigned world_seed,
                        int tx, int ty, int tz);

// (re)compute the three offers for the item currently placed. call after the
// item or surrounding shelves change. clears any TAKEN flags.
void enchant_table_refresh(enchant_table *t, world *w, int tx, int ty, int tz,
                           enchant_cat item_cat, int enchantability);

// can the player afford and is the slot live? player_level is their xp level.
int  enchant_table_can_take(const enchant_table *t, int slot, int player_level);

// take a slot: marks it TAKEN, returns the rolled set via *out and the xp cost
// via *cost. returns 1 on success, 0 if not takeable.
int  enchant_table_take(enchant_table *t, int slot, enchant_set *out, int *cost);

#endif
