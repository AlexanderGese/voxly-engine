#ifndef ENTITY_NAVMESH_NAV_FUNNEL_H
#define ENTITY_NAVMESH_NAV_FUNNEL_H
#include "nav_path.h"
#include "../../world/world.h"
// path smoothing. nav_path_find hands back one waypoint per cell, dead-centre
// of every tile, which makes a mob walk a staircase of right-angle turns even
// across open ground. this pass pulls the string taut: it drops any waypoint
// you could have walked straight past, leaving only the corners that matter.
//
// we do it the cheap way for a voxel world — a greedy line-of-sight skip over
// the world's collision rather than a true portal funnel — because our "cells"
// are points, not edges, so there's no portal to thread. close enough, and a
// lot less code than a proper simple-stupid-funnel.
// the most waypoints a smoothed path can keep. never more than the input had.
#define NAV_FUNNEL_MAX  NAV_PATH_MAX
// sample step for the los walk, in blocks. smaller = more accurate, slower.
// a third of a block catches diagonal block corners without being silly.
#define NAV_FUNNEL_STEP  0.34f
// is the straight segment a->b walkable for a `width`-wide agent? we march
// the segment and reject if any sample hits a non-standable column. this is
// what decides whether a corner can be skipped. exposed because the mover's
int nav_funnel_clear(world *w, vec3 a, vec3 b, float width);
int nav_funnel_smooth(world *w, const nav_path *in, nav_path *out, float width);
#endif
