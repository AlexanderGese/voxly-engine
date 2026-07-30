#ifndef ENTITY_NAVMESH_NAV_LINK_H
#define ENTITY_NAVMESH_NAV_LINK_H
#include "nav_grid.h"
#include "../../world/world.h"
// neighbour linking. for every cell we look at its four cardinal columns and,
// if there's a reachable floor there within the step rules, add a directed
// link. links are directed but we add them both ways when symmetric (a walk
// across is mutual; a big drop is one-way until we prove the agent can climb
// back). diagonals are intentionally skipped: cutting a corner over a block
int nav_link_step(nav_grid *g, world *w, int src, int dx, int dz);
void nav_link_all(nav_grid *g, world *w);
int nav_link_cost(int kind);
#endif
