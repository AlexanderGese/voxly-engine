#ifndef PLAYER_FISHING_LOOT_H
#define PLAYER_FISHING_LOOT_H

#include "fishing_types.h"
#include "fishing_rng.h"
#include "fishing_rod.h"

// the loot tables and the weighted roll over them. three pools (fish, treasure,
// junk); we first pick a pool by category odds, then a row within it. luck
// bends both steps toward treasure. the engine only ships a handful of block
// ids so the "fish" are stand-ins, but the weighting machinery is real.

// fixed per-category pull chance before luck, summing to 1. treasure is rare.
#define FISHING_BASE_FISH      0.85f
#define FISHING_BASE_TREASURE  0.05f
#define FISHING_BASE_JUNK      0.10f

// fetch the static table for a category. count is written to *n. never NULL.
const fishing_loot_entry *fishing_loot_table(fishing_catch_category cat, int *n);

// pick a category honouring luck. treasure odds scale up, junk down, fish takes
// the slack. always returns a real (non-NONE) category.
fishing_catch_category fishing_loot_pick_category(fishing_rng *r, const fishing_rod *rod);

// roll a single entry out of a category's table by weight. returns a resolved
// catch with a rolled stack count. category is the one passed in.
fishing_catch fishing_loot_roll_entry(fishing_rng *r, fishing_catch_category cat);

// the whole pull: pick a category, then an entry. the one call sites use.
fishing_catch fishing_loot_roll(fishing_rng *r, const fishing_rod *rod);

// the odds a single roll lands a given entry, all factors folded in: the luck-
// adjusted category chance times the entry's share of its table's weight. handy
// for a "drop rates" screen and for sanity-checking the tuning. returns 0..1.
float fishing_loot_entry_chance(const fishing_rod *rod,
                                fishing_catch_category cat, int entry_index);

// 0..3 rarity bucket for a category, for tinting the catch toast. treasure is
// the only genuinely rare one; fish/junk read as common.
int   fishing_loot_rarity(fishing_catch_category cat);

#endif
