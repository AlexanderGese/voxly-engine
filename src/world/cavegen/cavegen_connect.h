#ifndef WORLD_CAVEGEN_CONNECT_H
#define WORLD_CAVEGEN_CONNECT_H

#include "cavegen_grid.h"
#include "cavegen_types.h"

// connectivity pass. after the automata + worms run we have a bunch of open
// pockets, some of which are tiny sealed bubbles that look bad and trap mobs.
// this flood-fills the open space into labelled regions, measures each one, and
// seals (refills) anything below params.min_region_cells. it also reports the
// biggest region so callers can guarantee the player spawn tunnel stays open.

typedef struct {
    int region_count;     // labelled regions found
    int sealed_regions;   // how many got refilled for being too small
    int sealed_cells;     // total cells refilled
    int largest_cells;    // size of the biggest surviving region
    int largest_label;    // its label id (1-based, 0 means none)
} cavegen_connect_stats;

// run the full pass. mutates the grid in place: small pockets become SEALED,
// everything else stays open. returns the stats.
cavegen_connect_stats cavegen_connect_run(cavegen_grid *g, const cavegen_params *p);

// flood from a single seed cell over open neighbours, stamping `label` into the
// labels buffer. returns the cell count of the filled region. exposed for tests.
int cavegen_connect_flood(const cavegen_grid *g, int *labels,
                          int sx, int sy, int sz, int label);

#endif
