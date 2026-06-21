#ifndef WORLD_MINESHAFT_MAZE_H
#define WORLD_MINESHAFT_MAZE_H

#include "mineshaft_grid.h"
#include "mineshaft_rand.h"
#include "mineshaft_types.h"

// the carver. takes an initialized grid and an rng, runs a randomized
// depth-first growth from the entry cell, then braids in a few extra links so
// the result isn't a perfect tree (real shafts loop back and branch messily).
// classify() then tags each visited cell with a kind the builder dispatches on
// and computes bfs depth so the deepest cell can host the vertical shaft.

void mineshaft_maze_carve(mineshaft_grid *g, const mineshaft_config *cfg,
                          mineshaft_rng *rng);

// second pass: assign cell kinds from link topology and roll for ore rooms /
// cave-ins. split out so it can re-run without re-carving. uses `rng` directly.
void mineshaft_maze_classify(mineshaft_grid *g, const mineshaft_config *cfg,
                             mineshaft_rng *rng);

// flood bfs from the entry cell over carved links, filling cell.depth. returns
// the max depth reached (cell furthest from entry).
int  mineshaft_maze_depths(mineshaft_grid *g);

#endif
