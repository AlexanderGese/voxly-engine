#ifndef ENTITY_NAVMESH_NAV_ISLAND_H
#define ENTITY_NAVMESH_NAV_ISLAND_H

#include "nav_grid.h"
#include "../../math/vec3.h"

// region summaries. nav_region stamps every cell with a connected-component
// id; this rolls those up into one record per region: how big it is, where its
// rough centre sits, and its block-space bounds. the spawner uses it to pick a
// spawn point on a reachable island, and the ai overlay uses the centroid to
// label regions in the world.
//
// it's a derived view, recomputed from the grid on demand. cheap: one linear
// pass over the cells.

// hard cap on regions we'll summarise. matches the practical region count of a
// patch; anything past this gets lumped into the last bucket by the flood, so
// summarising past it is pointless.
#define NAV_MAX_ISLANDS  256

typedef struct {
    uint16_t region;     // the region id this summarises
    int      cells;      // how many cells belong to it
    int      sx, sz;     // running coord sums, used to derive the centroid
    int16_t  min_x, min_z, max_x, max_z;   // block-space xz extent
    int16_t  floor_y;    // a representative floor height (first cell seen)
} nav_island;

typedef struct {
    nav_island items[NAV_MAX_ISLANDS];
    int        count;
} nav_island_set;

// build the summary set from a flooded grid. clears `set` first. returns the
// number of distinct islands found (== set->count).
int nav_island_build(nav_island_set *set, const nav_grid *g);

// look up the summary for a region id, or NULL if it isn't in the set.
const nav_island *nav_island_get(const nav_island_set *set, int region);

// world-space centroid of an island (centre of its cells, on the floor). used
// for "spawn somewhere on this island" and overlay labels. returns origin if
// the island has no cells (shouldn't happen for a valid id).
vec3 nav_island_centroid(const nav_island *isl);

// pick the largest island in the set — the main walkable surface, usually the
// ground the player stands on. returns its region id, or NAV_REGION_NONE if
// the set is empty. great default target for "put a mob on solid ground".
int nav_island_largest(const nav_island_set *set);

#endif
