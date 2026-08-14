#ifndef PLAYER_TOOLS_XP_H
#define PLAYER_TOOLS_XP_H

#include "tools_types.h"

// the mining xp pool. the drop code hands back an xp amount per broken ore;
// this tracks the running total, the level it maps to, and the bar fraction the
// hud draws. levels cost progressively more, classic rpg curve.

typedef struct {
    int   total;        // lifetime xp gathered
    int   level;        // derived level, cached
    int   pool;         // xp sitting in the current level's bar
    int   to_next;      // xp needed to clear the current level
} xp_state;

void  tools_xp_init(xp_state *xs);

// xp required to advance *from* a given level to the next. grows with level.
int   tools_xp_for_level(int level);

// add raw xp (e.g. the return of tools_drop_resolve). recomputes level/pool and
// returns how many levels were gained this call (often 0).
int   tools_xp_add(xp_state *xs, int amount);

// spend xp, e.g. for an enchant. returns 1 if affordable and deducts, else 0.
int   tools_xp_spend(xp_state *xs, int amount);

// 0..1 fill of the current level bar, for the hud.
float tools_xp_bar_frac(const xp_state *xs);

#endif
