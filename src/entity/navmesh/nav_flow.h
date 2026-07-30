#ifndef ENTITY_NAVMESH_NAV_FLOW_H
#define ENTITY_NAVMESH_NAV_FLOW_H
#include "nav_grid.h"
#include "../../math/vec3.h"
// goal-oriented flow field over the cell graph. when a dozen zombies all want
// the same player, running a separate A* per mob is wasteful — they're all
// heading to one place. instead we run one reverse dijkstra from the goal cell
// and bake a "next hop toward goal" into every cell. each mob then reads its
// own cell's next-hop in O(1), no per-mob search at all.
//
// the field is computed against link costs run *backwards* (we walk incoming
// links), so a cell's distance is the cost to reach the goal from it. one-way
// drops are honoured: a cell above a big drop can flow down to the goal but the
// goal's field won't push anything back up, which is correct.
typedef struct {
    int   *dist;       // cost-to-goal per cell, INT_MAX if unreachable. owned.
    int   *next;       // next cell idx on the way to goal, -1 if none. owned.
    int    count;      // matches the grid cell count at build time
    int    goal;       // the cell the field flows toward
} nav_flow;
void nav_flow_init(nav_flow *f);
void nav_flow_free(nav_flow *f);
// (re)build the field so every reachable cell points toward `goal`. sizes the
int nav_flow_build(nav_flow *f, nav_grid *g, int goal);
vec3 nav_flow_step(const nav_flow *f, const nav_grid *g, int from, vec3 fallback);
int  nav_flow_reachable(const nav_flow *f, int from);
#endif
