#ifndef ENTITY_NAVMESH_NAV_REGION_H
#define ENTITY_NAVMESH_NAV_REGION_H

#include "nav_grid.h"

// connected-component labelling over the link graph. once cells are linked we
// flood the graph and stamp every cell with a region id. two cells share a
// region iff a path of links connects them (treating links as undirected for
// the flood, even one-way drops, since the components are about "is this the
// same island of floor", not "can i get back").
//
// the planner uses this as a fast reject: if start and goal sit in different
// regions there's no point running A*, the answer is already no.

// flood the whole grid. assigns region ids 1..N (0 stays reserved for
// "unlabelled"). returns the number of distinct regions found.
int nav_region_flood(nav_grid *g);

// quick same-island test. both indices must be valid cells. returns 0 if
// either is unlabelled.
int nav_region_connected(const nav_grid *g, int a, int b);

#endif
