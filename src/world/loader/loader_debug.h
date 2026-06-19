#ifndef WORLD_LOADER_DEBUG_H
#define WORLD_LOADER_DEBUG_H

#include "loader.h"

// readout helpers for the F3-style debug overlay. nothing here mutates the
// loader; it just reduces the ring + stats into numbers a human can read while
// running around watching chunks pop in.

// how many slots sit at each pipeline stage. counts[stage] for stage in
// LOADER_STAGE_EMPTY..RESIDENT. handy to eyeball whether the bottleneck is gen,
// light or mesh -- whichever stage is piling up is the slow one.
typedef struct {
    int per_stage[6];
    int occupied;
    int resident;
    int in_cooldown;
} loader_ring_census;

void loader_debug_census(const loader *l, loader_ring_census *out, uint64_t now_us);

// one-line summary into `buf`. returns the length written (excluding NUL). safe
// against truncation. format is terse on purpose, it shares a corner with the fps.
int  loader_debug_oneline(const loader *l, uint64_t now_us, char *buf, int cap);

// dump the full stats block to the log at INFO. call on a keybind, not per frame.
void loader_debug_dump(const loader *l);

#endif
