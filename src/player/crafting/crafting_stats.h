#ifndef PLAYER_CRAFTING_STATS_H
#define PLAYER_CRAFTING_STATS_H

#include "crafting_types.h"

// per-recipe usage stats + a recently-crafted ring. drives the "most used"
// sort in the book and the little recent-recipes strip. parallel arrays keyed
// by recipe id, same fixed cap as the book.

#define CRAFT_STATS_CAP    256
#define CRAFT_RECENT_RING  8     // last N distinct recipes crafted

void craft_stats_init(void);
void craft_stats_reset(void);

// record one craft of recipe `id` producing `made` items. bumps counters and
// pushes onto the recent ring (de-duped: re-crafting the same thing moves it
// to the front rather than filling the ring with one recipe).
void craft_stats_record(int recipe_id, int made);

// how many times recipe `id` has been crafted, and total items it produced.
int  craft_stats_times(int recipe_id);
int  craft_stats_yield(int recipe_id);

// the single most-crafted recipe id, or -1 if nothing's been crafted yet.
int  craft_stats_favorite(void);

// fill `out` (caller-sized `cap`) with recipe ids sorted by craft count desc.
// returns the number written. ties keep ascending id order (stable-ish).
int  craft_stats_top(int *out, int cap);

// copy the recent ring newest-first into `out` (caller-sized `cap`). returns
// count written.
int  craft_stats_recent(int *out, int cap);

// grand total of every craft ever, across all recipes. for an achievement-y
// "items crafted" counter.
int  craft_stats_total_crafts(void);

#endif
