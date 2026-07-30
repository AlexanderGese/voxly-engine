#ifndef ENTITY_PATHFIND_PF_H
#define ENTITY_PATHFIND_PF_H

#include "pf_types.h"
#include "pf_heuristic.h"
#include "../../math/vec3.h"
#include "../../world/world.h"

// public face of the pf_* pathfinder. one planner object holds the reusable
// scratch (grid window, node pool, heap) so a mob can keep it around and not
// thrash the allocator every replan. opaque on purpose; internals live in
// pf.c and the pf_* units it pulls in.
//
// this is the grown-up cousin of entity/pathfind_astar.c. the old one is
// fine for a zombie shuffling 5 blocks; this one handles jumps, drops,
// diagonals and produces smoothed waypoints.

typedef struct pf_planner pf_planner;

typedef struct {
    pf_heuristic_kind heuristic;
    int   allow_diagonal;
    int   smooth;            // run string-pull on the result
    int   max_expansions;    // 0 = use the default cap
} pf_options;

pf_options pf_options_default(void);

// create / destroy a reusable planner. allocates the node pool up front.
pf_planner *pf_planner_create(world *w);
void        pf_planner_destroy(pf_planner *p);

// plan a path between two world-space points. fills `out`. returns 1 if a
// usable path was produced (out->found set), 0 otherwise. even on 0 the
// path is left in a safe, empty state.
int pf_plan(pf_planner *p, vec3 from, vec3 to, const pf_options *opt, pf_path *out);

// path-follow helpers, mirror the little astar.c api so callers can swap.

// world-space target the entity should currently steer toward. advances the
// internal cursor when the entity gets close to the active waypoint.
vec3 pf_path_step(pf_path *path, vec3 entity_pos, float reach);

// has the entity consumed every waypoint?
int  pf_path_done(const pf_path *path);

#endif
