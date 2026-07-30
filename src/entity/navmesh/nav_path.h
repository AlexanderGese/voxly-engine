#ifndef ENTITY_NAVMESH_NAV_PATH_H
#define ENTITY_NAVMESH_NAV_PATH_H

#include "nav_grid.h"
#include "../../math/vec3.h"

// A* over the cell link graph. this is the payoff of the whole subsystem:
// because cells and links are already baked, a search is just a graph walk
// with no world pokes at all, which is dramatically cheaper than the pf_*
// block-level planner when the terrain is static.
//
// the heuristic is octile-ish distance between cell floor coords. link costs
// come from nav_link_cost so jumps and drops are discouraged.

#define NAV_PATH_MAX  128

typedef struct {
    vec3 pts[NAV_PATH_MAX];   // world-space waypoints, start -> goal
    int  count;
    int  cursor;              // which waypoint the mover is walking toward
    int  found;               // 1 = real path, 0 = no route / over budget
} nav_path;

// search from cell `start` to cell `goal` within `g`. fills `out`. returns 1
// if a path was found. a region mismatch short-circuits to 0 without searching.
int nav_path_find(nav_grid *g, int start, int goal, nav_path *out);

// search by world position: resolves start/goal cells, then calls the above.
// returns 0 if either endpoint can't be snapped onto the mesh.
int nav_path_find_pos(nav_grid *g, vec3 from, vec3 to, nav_path *out);

// advance the cursor if the mover reached the current waypoint, and return the
// next world target. returns the last point once the path is walked out.
vec3 nav_path_next(nav_path *p, vec3 mover_pos);

#endif
