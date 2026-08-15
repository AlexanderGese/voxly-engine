#ifndef PLAYER_XP_XP_STATE_H
#define PLAYER_XP_XP_STATE_H

// the actual per-player experience bookkeeping. one of these lives next to
// the player struct. it owns nothing fancy; just the numbers.

#include <stdint.h>

typedef struct {
    int64_t total;        // lifetime accumulated xp
    int     level;        // derived from total, cached
    int     prog_xp;      // xp into the current level
    int     prog_need;    // xp width of the current level
    float   prog_frac;    // prog_xp / prog_need, for the bar

    // transient bookkeeping for the frame
    int     last_gained;  // xp added this update (0 if none)
    int     levels_up;    // how many level boundaries we crossed last update
    int     pending_levelup; // set when we crossed >=1 boundary, cleared by reward dispatch
} xp_state;

void    xp_state_init(xp_state *s);

// add a chunk of xp. recomputes level/progress and flags level-ups. negative
// amounts are clamped to zero (no, dying does not refund your grind).
void    xp_state_add(xp_state *s, int amount);

// forcibly set the player to a level (debug / commands). progress resets to 0.
void    xp_state_set_level(xp_state *s, int level);

// clear the per-frame transients. called at the top of each update.
void    xp_state_tick_begin(xp_state *s);

// recompute cached level/progress from `total`. used after add or set.
void    xp_state_recompute(xp_state *s);

#endif
