#ifndef ENTITY_NAVMESH_NAV_BUILD_H
#define ENTITY_NAVMESH_NAV_BUILD_H

#include "nav_grid.h"
#include "../../world/world.h"

// the bake. given a world and a patch address (in chunk coords), scan the
// walkable surface across the patch + apron, drop cells, link them, and flood
// regions. the result is a self-contained nav_grid the queries run against.
//
// this is the only thing the rest of the engine should need to call to get a
// navmesh; everything else (cell/link/region/grid) is plumbing.

// vertical scan window. we don't bake the whole 128-tall column, just a band
// around sea level + a margin, which is where mobs and players actually walk.
// caves below get their own patches when something asks to navigate there.
#define NAV_SCAN_Y_MARGIN  40

typedef struct {
    int   pcx, pcz;      // patch address: the chunk coord of the patch min
    int   cells;         // how many cells the bake produced
    int   regions;       // distinct connected components
    int   partial;       // 1 if we hit the cell cap and bailed early
} nav_build_stats;

// bake a patch whose min chunk is (pcx, pcz). `g` must already be init'd (or
// reset). fills `stats` (may be NULL). returns the cell count.
int nav_build_patch(nav_grid *g, world *w, int pcx, int pcz,
                    nav_build_stats *stats);

#endif
