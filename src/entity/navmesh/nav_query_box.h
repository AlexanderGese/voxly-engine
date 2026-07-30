#ifndef ENTITY_NAVMESH_NAV_QUERY_BOX_H
#define ENTITY_NAVMESH_NAV_QUERY_BOX_H
#include "nav_grid.h"
#include "../../math/vec3.h"
#include "../../math/aabb.h"
// area queries against a baked grid. nav_query is point-ish (cell at / nearest
// to a position); this is the bulk side: collect every cell in a box, sample a
// random walkable cell for a spawn, or find the nearest cell that's also on a
// required region. spawners and wandering ai lean on these.
//
// read-only, like the rest of the query layer.
// gather up to `max` cell indices whose floor falls inside `box` (xz inside,
// floor y within the box's y span). returns how many were written. order is
// grid order, not spatial — caller can shuffle if it wants randomness.
int nav_query_box(const nav_grid *g, aabb box, int *out, int max);
// count cells inside the box without collecting them. cheap pre-check for
// "is there anywhere to stand around here".
int nav_query_box_count(const nav_grid *g, aabb box);
// pick a pseudo-random walkable cell inside the box, biased to nothing in
// particular. `rng_state` is advanced in place (a tiny xorshift so we don't
// drag the engine rng in). returns a cell idx or -1 if the box holds none.
int nav_query_box_random(const nav_grid *g, aabb box, uint32_t *rng_state);
// nearest cell to world point `p` that also sits in region `region`. like
// nav_query_nearest but constrained, for "send this mob to the nearest spot on
int nav_query_nearest_in_region(const nav_grid *g, vec3 p, int region, int r);
#endif
