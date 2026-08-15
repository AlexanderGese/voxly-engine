#ifndef PLAYER_XP_XP_PERK_H
#define PLAYER_XP_XP_PERK_H

// passive stat modifiers unlocked by leveling. each perk nudges one derived
// stat. they stack additively per-stat; the aggregate is what the rest of
// the player code would read (move speed mult, extra reach, etc).
//
// nothing here reaches into the player struct directly. the facade exposes
// the totals; wiring them into movement/combat is the caller's job.

#include "xp_config.h"

typedef enum {
    XP_STAT_MOVE_SPEED,   // multiplier, base 1.0
    XP_STAT_REACH,        // additive blocks onto PLAYER_REACH
    XP_STAT_MAX_HP,       // additive hp onto PLAYER_MAX_HP
    XP_STAT_MINE_SPEED,   // multiplier on break speed
    XP_STAT_MAGNET,       // multiplier on orb magnet range
    XP_STAT_COUNT
} xp_stat;

typedef struct {
    xp_stat stat;
    float   amount;   // interpreted per-stat (mult delta or flat add)
    int     from_level; // level that granted it, for display
} xp_perk;

typedef struct {
    xp_perk perks[XP_PERK_SLOTS];
    int     count;

    // cached aggregate so consumers don't re-fold every frame.
    float   totals[XP_STAT_COUNT];
} xp_perk_set;

void  xp_perk_init(xp_perk_set *set);

// add a perk. if full, the weakest perk for that stat is evicted (we keep
// the strong ones). recomputes totals. returns 1 if stored, 0 if dropped.
int   xp_perk_add(xp_perk_set *set, xp_stat stat, float amount, int from_level);

// resolved value for a stat. multiplier stats start at 1.0, additive at 0.0.
float xp_perk_total(const xp_perk_set *set, xp_stat stat);

// is this stat a multiplier (vs additive)? drives base value + clamping.
int   xp_perk_is_mult(xp_stat stat);

#endif
