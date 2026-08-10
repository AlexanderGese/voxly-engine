#ifndef PLAYER_ENCHANT_REGISTRY_H
#define PLAYER_ENCHANT_REGISTRY_H

#include "enchant_types.h"

// the static table of enchant kinds. populated once at startup from a
// hardcoded list (no data files for this; the recipe loader is the only
// thing that earns a parser around here). lookups are by id and are O(1)
// because ids are dense and we index straight in.

// build the registry. idempotent; calling twice just re-stamps the same
// rows. returns the number of kinds registered.
int                enchant_registry_init(void);

// total registered kinds, including ENCHANT_NONE? no — excluding none.
int                enchant_registry_count(void);

// fetch a definition by id. returns NULL for ENCHANT_NONE or any id past the
// table. never returns a dangling pointer; the table is static storage.
const enchant_def *enchant_registry_get(enchant_id id);

// iterate by dense index 0..count-1. handy for the roll picker which walks
// every kind weighing them.
const enchant_def *enchant_registry_at(int index);

// look up by name, case sensitive. linear scan; only the debug console uses
// it so we don't bother indexing. returns NULL if unknown.
const enchant_def *enchant_registry_find(const char *name);

// does this enchant apply to an item of the given category mask?
int                enchant_applies_to(enchant_id id, enchant_cat item_cat);

#endif
