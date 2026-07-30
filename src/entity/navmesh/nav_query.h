#ifndef ENTITY_NAVMESH_NAV_QUERY_H
#define ENTITY_NAVMESH_NAV_QUERY_H

#include "nav_grid.h"
#include "../../math/vec3.h"

// read-side queries against a baked grid. nothing here mutates the grid, so
// many movers can share one navmesh while it's static.

// find the cell an entity at world position `p` is standing on / nearest to.
// snaps p to a block column, then looks for a floor at or just below the
// entity's feet. returns the cell index or -1 if there's none nearby.
int nav_query_cell_at(const nav_grid *g, vec3 p);

// like above but searches outward in a small ring if the exact column misses,
// which it often does when an entity is mid-jump or wedged in a corner. `r`
// is the search radius in blocks. returns -1 if nothing within r.
int nav_query_nearest(const nav_grid *g, vec3 p, int r);

// can an agent get from cell `a` to cell `b` at all? pure region check, O(1),
// no search. a true here means A* will succeed; false means don't bother.
int nav_query_reachable(const nav_grid *g, int a, int b);

// snap a world position to the centre of its nav cell, or return p unchanged
// if there's no cell. handy for clamping a mob to the mesh.
vec3 nav_query_snap(const nav_grid *g, vec3 p);

#endif
