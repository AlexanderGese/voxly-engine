#ifndef ENTITY_PATHFIND_PF_HEURISTIC_H
#define ENTITY_PATHFIND_PF_HEURISTIC_H

#include "pf_types.h"

// heuristics, all in the same fixed-point scale as pf_node costs (card=10,
// diag=14). they must stay admissible (never over-estimate) or A* stops
// finding shortest paths. octile is the tightest one for 8-connected grids
// so it's the default; the others are here mostly for tuning/debug.

typedef enum {
    PF_H_MANHATTAN = 0,   // cheap, over-estimates diagonals slightly -> faster, not optimal
    PF_H_OCTILE,          // exact for 8-connected, admissible. default.
    PF_H_EUCLIDEAN,       // smooth, admissible, a touch slower
    PF_H_CHEBYSHEV        // diagonal distance, ignores card/diag ratio
} pf_heuristic_kind;

// estimate remaining cost from a -> b. vertical distance is folded in with
// a light weight so the search prefers staying level.
int pf_heuristic(pf_heuristic_kind kind, pf_coord a, pf_coord b);

#endif
