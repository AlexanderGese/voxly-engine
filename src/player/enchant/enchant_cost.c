#include "enchant_cost.h"

#include <stddef.h>

int enchant_cost_dye_for_slot(int slot) {
    if (slot < 0) slot = 0;
    if (slot >= ENCHANT_TABLE_SLOTS) slot = ENCHANT_TABLE_SLOTS - 1;
    return slot + 1;
}

int enchant_cost_xp_spent(int slot) {
    // you pay (slot+1) levels regardless of the inflated requirement.
    return enchant_cost_dye_for_slot(slot);
}

int enchant_cost_can_afford(int displayed_level, int slot,
                            int player_level, int player_dye) {
    if (player_level < displayed_level) return 0;
    if (player_dye < enchant_cost_dye_for_slot(slot)) return 0;
    // and you still need at least (slot+1) levels actually present to spend.
    if (player_level < enchant_cost_xp_spent(slot)) return 0;
    return 1;
}

int enchant_cost_xp_for_level(int level) {
    if (level <= 0) return 0;
    // a tame quadratic curve: cumulative points to reach `level` is roughly
    // level*(level+6). cheap to evaluate and monotone, which is all the
    // wallet needs.
    return level * (level + 6);
}

void enchant_cost_spend_levels(int levels, int *cur_level, int *cur_points) {
    if (!cur_level || !cur_points || levels <= 0) return;

    int lvl = *cur_level;
    int pts = *cur_points;

    // burn `levels` whole levels. when the current level runs out of points we
    // step down a level and refill `pts` from that level's width. the width of
    // a level is the derivative of the curve, approximated as 2*level+7.
    for (int i = 0; i < levels && lvl > 0; ++i) {
        if (lvl <= 0) break;
        lvl--;
        // dropping a level resets the in-level progress to that level's top.
        int width = 2 * lvl + 7;
        pts = width > 0 ? width - 1 : 0;
        (void)pts; // pts here represents progress into the new (lower) level
    }
    if (lvl < 0) lvl = 0;

    *cur_level  = lvl;
    *cur_points = lvl > 0 ? pts : 0;
}
