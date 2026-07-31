#ifndef ENTITY_PATHFIND_PF_SMOOTH_H
#define ENTITY_PATHFIND_PF_SMOOTH_H

#include "pf_astar.h"

// path smoothing. raw A* paths zig-zag along the grid; mobs walking those
// look drunk. we do a "string pull": greedily skip waypoints we can reach
// in a straight walkable line, then optionally chamfer the remaining
// corners so the motion reads less robotic.

// collapse runs of collinear points and string-pull over clear lines.
// writes block-coord points into `out` (same buffer type as raw).
// returns the resulting count.
int pf_smooth_string_pull(pf_grid *g, const pf_rawpath *in, pf_rawpath *out);

// drop intermediate points that lie (within eps) on the line between their
// neighbours. run before string-pull as a cheap first pass.
int pf_smooth_collinear(const pf_rawpath *in, pf_rawpath *out);

// is the straight 2d walk between two block columns clear the whole way?
// supercover line walk, checking standability at each crossed cell.
int pf_smooth_line_clear(pf_grid *g, pf_coord a, pf_coord b);

#endif
