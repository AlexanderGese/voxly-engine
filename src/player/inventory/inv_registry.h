#ifndef PLAYER_INVENTORY_REGISTRY_H
#define PLAYER_INVENTORY_REGISTRY_H

#include "inv_types.h"

// the item registry. one process-wide table mapping inv_item_id -> def.
// register everything up front at boot, then it's read-only for the rest of
// the run. lookups are O(1) array indexing; the name->id path is a linear
// scan because the table is tiny and it only runs from console/save code.

#define INV_MAX_ITEMS  512

// build the default table (blocks get auto-registered as placeable items,
// plus a handful of tools/materials). idempotent: calling twice resets.
void inv_registry_init(void);

// register a new item, returns its id. returns INV_ITEM_NONE if the table is
// full. max_stack of 0 is treated as 1 (unstackable). durability>0 forces the
// stack to 1 no matter what category says.
inv_item_id inv_registry_add(const char *name, inv_category cat,
                             uint16_t max_stack, block_id place_block,
                             int atlas_tile, uint16_t durability);

// borrow the def for an id. never NULL for a valid id; returns the NONE def
// (a zeroed sentinel) for out-of-range ids so callers can avoid branching.
const inv_item_def *inv_registry_get(inv_item_id id);

// convenience accessors that fall through to the def.
uint16_t    inv_item_max_stack(inv_item_id id);
inv_category inv_item_category(inv_item_id id);
int         inv_item_stackable(inv_item_id id);   // max_stack > 1
const char *inv_item_name(inv_item_id id);

// reverse lookups. id 0 if not found.
inv_item_id inv_registry_find(const char *name);
inv_item_id inv_registry_for_block(block_id b);   // item that places this block

int inv_registry_count(void);

#endif
