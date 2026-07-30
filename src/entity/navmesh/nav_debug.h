#ifndef ENTITY_NAVMESH_NAV_DEBUG_H
#define ENTITY_NAVMESH_NAV_DEBUG_H

#include "nav_grid.h"
#include "nav_path.h"
#include "../../render/debugdraw/debugdraw.h"

// visualise a baked grid through the engine's debug drawer. lives under the
// DD_CAT_AI category so the f3 menu can toggle it with the rest of the ai
// overlays. none of this runs unless someone calls it, so it's free in a
// shipping build.
//
// the colour-by-region trick makes disconnected islands obvious at a glance,
// which is the single most useful thing when a mob "can't reach" something and
// you want to know if it's a region split or a planner bug.

// draw a little marker on every cell, tinted by its region id so each
// connected island gets its own hue. cells sit half a block above their floor
// so they hover over the tile they stand on.
void nav_debug_cells(debugdraw *dd, const nav_grid *g);

// draw the link graph as line segments between cell centres. walk links in
// one colour, jumps/drops in another so you can see where the risky moves are.
// skips the back-edge of a symmetric pair so we don't double-draw every line.
void nav_debug_links(debugdraw *dd, const nav_grid *g);

// outline just the cells belonging to one region. handy for confirming a
// reachability query landed where you think it did.
void nav_debug_region(debugdraw *dd, const nav_grid *g, int region, ddcolor c);

// draw a planned path as a polyline with node markers, hovering slightly so it
// reads above the cell markers. uses the path's `found` flag to pick colour:
// green for a real route, red for a failed one.
void nav_debug_path(debugdraw *dd, const nav_path *p);

#endif
