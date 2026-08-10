#ifndef PLAYER_ENCHANT_ROLL_H
#define PLAYER_ENCHANT_ROLL_H

#include "enchant_types.h"
#include "../../math/rng.h"

// the weighted roll. given an enchant "level" (the slot number 1..30) and an
// item category, produce a set of enchants. this is where the table earns
// its keep: a single slot can roll multiple enchants, biased toward the ones
// the item is compatible with and weighted by rarity.

// modifies the effective level with the same triangular noise the source
// material uses, so two identical slots don't always give the same spread.
// exposed for the table preview which wants to show the modified band.
int enchant_roll_modified_level(rng *r, int base_level, int enchantability);

// pick a single weighted enchant compatible with item_cat at the given
// modified level. returns ENCHANT_NONE if nothing is eligible. out_level
// receives the chosen enchant's level (1..max).
enchant_id enchant_roll_pick_one(rng *r, int modified_level,
                                 enchant_cat item_cat, int *out_level);

// roll a full result for a slot: picks a primary enchant, then probabilistic-
// ally tacks on extras at decaying odds, skipping anything that conflicts
// with what's already chosen. fills `out` (cleared first). returns the number
// of enchants placed.
int enchant_roll_slot(rng *r, int base_level, int enchantability,
                      enchant_cat item_cat, enchant_set *out);

#endif
