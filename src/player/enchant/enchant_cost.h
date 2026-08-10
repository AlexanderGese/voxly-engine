#ifndef PLAYER_ENCHANT_COST_H
#define PLAYER_ENCHANT_COST_H

#include "enchant_types.h"

// the resource side of enchanting: xp levels and lapis-equivalent dye. the
// table charges both. these helpers keep the bookkeeping in one place so the
// table ui and the player wallet code agree on the numbers.

// dye cost (our stand-in for lapis) to take a given table slot, 0-indexed.
// slot 0 costs 1, slot 1 costs 2, slot 2 costs 3, like the original.
int enchant_cost_dye_for_slot(int slot);

// the actual xp levels consumed when taking a slot. note this differs from the
// displayed "level requirement": you must *have* the requirement, but you only
// *spend* (slot+1) levels. classic gotcha, faithfully reproduced.
int enchant_cost_xp_spent(int slot);

// does the player meet the requirement to take a slot? needs both the level
// floor (the displayed number) and enough dye.
int enchant_cost_can_afford(int displayed_level, int slot,
                            int player_level, int player_dye);

// running total xp needed to reach a player level, using a simple quadratic
// xp curve. lets the wallet convert spent-levels into spent-points if it
// tracks raw xp instead of whole levels.
int enchant_cost_xp_for_level(int level);

// after spending `levels` whole levels starting from `cur_level` with
// `cur_points` into it, write back the remaining level/points. handles the
// borrow across level boundaries.
void enchant_cost_spend_levels(int levels, int *cur_level, int *cur_points);

#endif
