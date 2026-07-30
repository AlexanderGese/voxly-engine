#ifndef ENTITY_NAVMESH_NAV_COST_H
#define ENTITY_NAVMESH_NAV_COST_H

#include "nav_grid.h"
#include "nav_filter.h"
#include "../../math/vec3.h"

// cost model the planner reads. nav_link_cost gives a flat per-kind number;
// this layers an optional per-cell area cost on top so movers can be nudged
// away from places without us re-baking links. think: cows preferring grass,
// hostiles cutting straight lines, everyone avoiding the edge of a drop.
//
// the area cost is a parallel byte array indexed by cell. it's not part of the
// baked grid (a single bake can be shared by mobs with different preferences)
// so the caller owns it and passes it alongside the grid + filter.

// neutral area cost. links cost their base; nothing is preferred or avoided.
#define NAV_COST_NEUTRAL  1

typedef struct {
    uint8_t *area;     // owned, one byte per cell. NULL = treat all as neutral.
    int      count;    // length of `area`; should match the grid's cell count.
} nav_cost_field;

void nav_cost_init(nav_cost_field *cf, int cell_count);
void nav_cost_free(nav_cost_field *cf);

// reset every cell back to neutral.
void nav_cost_clear(nav_cost_field *cf);

// stamp an area cost multiplier (1..255) onto every cell within `radius`
// blocks of world point `p`. higher = more reluctant to walk there. used to
// paint a soft avoidance bubble around hazards (lava, the player, a campfire).
void nav_cost_brush(nav_cost_field *cf, const nav_grid *g,
                    vec3 p, float radius, uint8_t cost);

// total cost of taking link `li` out of cell `ci`, folding in the link kind,
// the area cost of the destination cell, and the filter's preferences. returns
// a large sentinel (NAV_COST_BLOCKED) if the filter forbids the link, so the
// planner can treat "not allowed" and "very expensive" uniformly.
#define NAV_COST_BLOCKED  100000

int nav_cost_link(const nav_cost_field *cf, const nav_filter *f,
                  const nav_grid *g, int ci, int li);

// octile-ish heuristic between two cells' floor coords, in the same units as
// the link costs. admissible: never overestimates a straight walk.
int nav_cost_heuristic(const nav_grid *g, int a, int b);

#endif
