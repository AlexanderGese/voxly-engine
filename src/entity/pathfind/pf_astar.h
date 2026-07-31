#ifndef ENTITY_PATHFIND_PF_ASTAR_H
#define ENTITY_PATHFIND_PF_ASTAR_H

#include "pf_types.h"
#include "pf_grid.h"
#include "pf_nodepool.h"
#include "pf_openset.h"
#include "pf_heuristic.h"

// the search core. owns nothing it can borrow: grid, pool and heap are all
// passed in so the caller can keep them alive and reuse them across calls.
// output is a raw block-coord path (no smoothing yet, that's pf_smooth).

#define PF_RAW_MAX  256

typedef struct {
    pf_coord pts[PF_RAW_MAX];
    int      count;
} pf_rawpath;

typedef struct {
    pf_grid     *grid;
    pf_nodepool *pool;
    pf_openset  *open;
    pf_heuristic_kind heuristic;
    int          max_expansions;   // hard cap so we never hang a frame
    int          allow_diagonal;
} pf_search;

// fill `s` with sane defaults pointing at the given borrowed structures.
void pf_search_setup(pf_search *s, pf_grid *grid, pf_nodepool *pool,
                     pf_openset *open);

// run A* from start to goal (block coords). on success fills `out` with the
// block path start..goal inclusive and returns 1. returns 0 if no path or
// the expansion cap was hit.
int  pf_astar_run(pf_search *s, pf_coord start, pf_coord goal, pf_rawpath *out);

#endif
