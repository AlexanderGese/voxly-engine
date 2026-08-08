#ifndef PLAYER_CRAFTING_STATS_H
#define PLAYER_CRAFTING_STATS_H
#include "crafting_types.h"
// per-recipe usage stats + a recently-crafted ring. drives the "most used"
// sort in the book and the little recent-recipes strip. parallel arrays keyed
#define CRAFT_STATS_CAP    256
#define CRAFT_RECENT_RING  8     // last N distinct recipes crafted
void craft_stats_init(void);
void craft_stats_reset(void);
void craft_stats_record(int recipe_id, int made);
int  craft_stats_times(int recipe_id);
int  craft_stats_yield(int recipe_id);
int  craft_stats_favorite(void);
int  craft_stats_top(int *out, int cap);
int  craft_stats_recent(int *out, int cap);
int  craft_stats_total_crafts(void);
#endif
