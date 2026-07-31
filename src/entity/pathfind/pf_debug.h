#ifndef ENTITY_PATHFIND_PF_DEBUG_H
#define ENTITY_PATHFIND_PF_DEBUG_H

#include "pf.h"
#include "pf_grid.h"
#include "pf_astar.h"

// path validation + cheap stats. handy when a mob walks into a wall and you
// need to know whether the planner lied or the controller fumbled. none of
// this runs in the hot path; it's for the debug overlay and the unit-ish
// sanity checks i poke at from main during dev.

typedef struct {
    int   waypoints;
    float total_length;   // summed world-space segment lengths
    int   direction_changes;
    int   max_step_up;
    int   max_step_down;
    int   valid;          // 1 = every segment stays walkable
} pf_path_stats;

// walk the finished waypoint path and gather stats. `g` must be a grid window
// that still covers the path (i.e. the same one used to plan it).
pf_path_stats pf_debug_stats(pf_grid *g, const pf_path *path);

// hard validity check: are all waypoints standable and are consecutive ones
// connected by a clear line? returns 1 if the path is actually walkable.
int pf_debug_validate(pf_grid *g, const pf_path *path);

// dump a path to the log via util/log. terse, one line per waypoint.
void pf_debug_dump(const pf_path *path);

// dump the walkability of the grid window as ascii so i can eyeball it.
// '.' standable, '#' blocked, 'S'/'G' start/goal columns. writes to log.
void pf_debug_dump_grid(pf_grid *g, pf_coord start, pf_coord goal);

#endif
