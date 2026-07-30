#ifndef ENTITY_NAVMESH_NAV_LINK_H
#define ENTITY_NAVMESH_NAV_LINK_H

#include "nav_grid.h"
#include "../../world/world.h"

// neighbour linking. for every cell we look at its four cardinal columns and,
// if there's a reachable floor there within the step rules, add a directed
// link. links are directed but we add them both ways when symmetric (a walk
// across is mutual; a big drop is one-way until we prove the agent can climb
// back). diagonals are intentionally skipped: cutting a corner over a block
// edge looks bad and lets mobs clip walls.

// add the single best link from cell `src` toward the column (dx,dz) away,
// poking the world for clearance so we don't link through a wall. returns 1
// if a link was added. needs `w` for the squeeze-through corner test.
int nav_link_step(nav_grid *g, world *w, int src, int dx, int dz);

// link every cell in the grid to its cardinal neighbours. call after all
// cells have been placed (links are by index, so the array must be stable).
void nav_link_all(nav_grid *g, world *w);

// cost of a link kind in the same units pf uses (roughly blocks). jumps and
// drops cost a bit more so the planner prefers flat ground when it can.
int nav_link_cost(int kind);

#endif
