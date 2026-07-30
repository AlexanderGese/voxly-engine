#ifndef ENTITY_NAVMESH_NAVMESH_H
#define ENTITY_NAVMESH_NAVMESH_H

#include "nav_grid.h"
#include "nav_build.h"
#include "../../world/world.h"
#include "../../util/hashmap.h"

// the public face of the navmesh. owns a small cache of baked patches keyed by
// patch address, bakes them lazily, and evicts the oldest when the cache is
// full. movers ask it "give me the mesh covering this position" and it hands
// back a grid, baking on demand.
//
// keeping more than a handful of patches resident is wasteful (each is a few
// hundred KB) so the cache is deliberately tiny. it's a working set for the
// mobs near the player, not a world-wide bake.

#define NAV_CACHE_PATCHES  8

typedef struct {
    nav_grid grid;
    int      pcx, pcz;     // patch address this grid was baked for
    int      valid;        // 0 = slot empty
    uint32_t stamp;        // lru clock value, higher = used more recently
} nav_patch;

typedef struct {
    world     *w;          // borrowed
    nav_patch  patches[NAV_CACHE_PATCHES];
    hashmap    addr;       // packed patch addr -> patch slot index + 1
    uint32_t   clock;      // monotonically increasing lru stamp source
    int        builds;     // how many bakes we've done, for stats/debug
} navmesh;

void navmesh_init(navmesh *nm, world *w);
void navmesh_free(navmesh *nm);

// drop all cached patches (e.g. after a big world edit). next query rebakes.
void navmesh_invalidate_all(navmesh *nm);

// invalidate just the patch covering world block (wx, wz), if it's cached.
// call this when blocks change so stale geometry doesn't linger.
void navmesh_invalidate_at(navmesh *nm, int wx, int wz);

// fetch (baking if needed) the grid whose patch contains world pos `p`.
// returns NULL only if the bake produced nothing usable.
nav_grid *navmesh_grid_for(navmesh *nm, vec3 p);

// convenience: resolve the cell an entity at `p` stands on, baking the patch
// under it if necessary. returns the grid in *g_out and the cell index, or -1.
int navmesh_cell_at(navmesh *nm, vec3 p, nav_grid **g_out);

// map a world block coord to its owning patch address (min chunk of patch).
void navmesh_patch_addr(int wx, int wz, int *pcx, int *pcz);

#endif
