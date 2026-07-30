#ifndef ENTITY_NAVMESH_NAV_LOS_H
#define ENTITY_NAVMESH_NAV_LOS_H

#include "nav_grid.h"
#include "../../math/vec3.h"

// line-of-walk over the baked mesh. given two cells, can an agent walk a
// straight line between them staying on connected floor the whole way? unlike
// nav_funnel (which marches the world's collision) this walks the *cell graph*
// — it's the mesh's own answer, used as a fast pre-check before committing to a
// world sweep, and to let the planner take shortcuts when the floor is convex.
//
// the walk is a 2d dda across the cell coords: step along the xz line one block
// at a time and confirm each block has a cell roughly at the expected height.
// no world pokes, so it's cheap, but it trusts the bake — if the mesh is stale
// it'll lie, same as everything else here.

// how far the floor under the line may deviate (in blocks of y) from the linear
// interpolation between the endpoints and still count as "on the line". covers
// gentle slopes; a cliff in the middle breaks the los, which is what we want.
#define NAV_LOS_Y_SLACK  2

// can an agent walk straight from cell `a` to cell `b` over connected floor?
// returns 1 if every block the line crosses has a cell within the y slack.
// endpoints must be valid cell indices.
int nav_los_cells(const nav_grid *g, int a, int b);

// world-position flavour: snaps both ends onto the mesh then runs the above.
// returns 0 if either point can't be snapped.
int nav_los_pos(const nav_grid *g, vec3 from, vec3 to);

// the furthest cell along the straight line from `a` toward `b` that still has
// clear line-of-walk. used to shortcut a path: jump the mover straight to the
// last visible cell instead of plodding through every waypoint. returns `a` if
// even the first step is blocked.
int nav_los_furthest(const nav_grid *g, int a, int b);

#endif
