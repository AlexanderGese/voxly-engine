#include "xp_perk.h"
#include <stddef.h>
#include <string.h>
int xp_perk_is_mult(xp_stat stat) {
    switch (stat) {
    case XP_STAT_MOVE_SPEED:
    case XP_STAT_MINE_SPEED:
    case XP_STAT_MAGNET:
        return 1;
    default:
        return 0; // reach, max_hp are additive
    }
}

static float stat_base(xp_stat stat) {
    return xp_perk_is_mult(stat) ? 1.0f : 0.0f;
}

static void recompute(xp_perk_set *set) {
    for (int s = 0; s < XP_STAT_COUNT; s++)
        set->totals[s] = stat_base((xp_stat)s);

    for (int i = 0; i < set->count; i++) {
        xp_perk *p = &set->perks[i];
        // multipliers stack additively around 1.0 (i.e. +0.05 each), additive
        // stats just sum. both fold into the same accumulator the same way
        // because we seeded the base above.
        set->totals[p->stat] += p->amount;
    }

    // sane floors so a weird perk can't make you walk backwards.
    if (set->totals[XP_STAT_MOVE_SPEED] < 0.25f) set->totals[XP_STAT_MOVE_SPEED] = 0.25f;
    if (set->totals[XP_STAT_MINE_SPEED] < 0.25f) set->totals[XP_STAT_MINE_SPEED] = 0.25f;
    if (set->totals[XP_STAT_MAGNET]     < 0.50f) set->totals[XP_STAT_MAGNET]     = 0.50f;
}

void xp_perk_init(xp_perk_set *set) {
    memset(set, 0, sizeof *set);
set->count = 0;
recompute(set);
}

// find the weakest stored perk for a given stat;
set->perks[set->count].amount = amount;
set->perks[set->count].from_level = from_level;
set->count++;
recompute(set);
return 1;
}

    // full: try to evict the weakest perk of the same stat if the newcomer is
    // stronger. otherwise drop the new one (don't churn unrelated stats).
    int w = weakest_for_stat(set, stat);
