#ifndef PLAYER_FISHING_STATS_H
#define PLAYER_FISHING_STATS_H

#include "fishing_types.h"

// a running tally of a player's fishing career plus a small ring of the most
// recent catches, for an achievements / "biggest catch" sort of readout. all
// integers, no allocation, lives wherever the player struct can hold it.

#define FISHING_HISTORY_LEN  16   // last N catches, wraps

typedef struct {
    fishing_catch_category category;
    block_id               block;
    int                    count;
} fishing_stats_entry;

typedef struct {
    // lifetime totals.
    int casts;                                  // lines put in the water
    int catches;                                // successful reel-ins
    int snaps;                                  // lines lost to tension
    int misses;                                 // bite windows whiffed
    int per_category[CATCH_CATEGORY_COUNT];     // catches by category
    int total_items;                            // sum of all catch counts

    // recent-catch ring.
    fishing_stats_entry history[FISHING_HISTORY_LEN];
    int head;                                   // next write slot
    int filled;                                 // how many of the ring are valid
} fishing_stats;

void  fishing_stats_init(fishing_stats *s);

// event hooks the session calls. cheap, just bump counters.
void  fishing_stats_on_cast(fishing_stats *s);
void  fishing_stats_on_miss(fishing_stats *s);
void  fishing_stats_on_snap(fishing_stats *s);
void  fishing_stats_on_catch(fishing_stats *s, const fishing_catch *c);

// derived readouts.
float fishing_stats_catch_rate(const fishing_stats *s);   // catches / casts, 0..1
int   fishing_stats_recent(const fishing_stats *s, int back, fishing_stats_entry *out);

#endif
