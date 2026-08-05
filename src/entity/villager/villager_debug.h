#ifndef ENTITY_VILLAGER_DEBUG_H
#define ENTITY_VILLAGER_DEBUG_H

#include "villager.h"
#include "villager_manager.h"

// small introspection helpers for the debug overlay / logging. no rendering
// here — just turning villager state into short human-readable strings and
// a couple of roll-up stats. handy when a villager gets stuck in a wall and
// you want to know what it *thinks* it's doing.

// short label for the current activity, e.g. "work". never NULL.
const char *villager_debug_activity_name(villager_activity a);

// one-line summary into `buf` (NUL-terminated). returns chars written
// (excluding the NUL), clamped to cap-1.
int villager_debug_describe(const villager *v, char *buf, int cap);

// village-wide tallies for an overlay readout.
typedef struct {
    int total;
    int babies;
    int employed;
    int sleeping;
    int panicking;
    int by_prof[VILLAGER_PROF_COUNT];
} villager_debug_stats;

void villager_debug_collect(const villager_manager *m, villager_debug_stats *out);

#endif
