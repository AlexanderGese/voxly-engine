#ifndef PLAYER_XP_XP_REWARD_H
#define PLAYER_XP_XP_REWARD_H

// level-up rewards. crossing a level boundary fires a reward; some are
// milestone-only (every 5th, every 10th...), most are a small perk roll.
// deterministic per-level so a given save always grants the same thing.

#include "xp_perk.h"

typedef enum {
    XP_RW_NONE = 0,
    XP_RW_PERK,        // granted a stat perk
    XP_RW_HEAL,        // full heal milestone
    XP_RW_MILESTONE,   // round-number celebration (cosmetic + perk)
} xp_reward_kind;

typedef struct {
    xp_reward_kind kind;
    int    level;       // the level reached
    xp_stat stat;       // valid when a perk was granted
    float  amount;      // perk magnitude
    int    heal;        // hp to restore (0 if none)
} xp_reward;

// compute the reward for reaching `level`. pure function of the level number
// (and a fixed internal seed), so it's stable and testable.
xp_reward xp_reward_for_level(int level);

// apply a reward to a perk set + optional heal sink. returns 1 if it granted
// a perk (so the caller can post an event), 0 otherwise. `out_heal` receives
// any hp to restore; pass null to ignore.
int  xp_reward_apply(const xp_reward *rw, xp_perk_set *perks, int *out_heal);

#endif
