#ifndef WORLD_FLUID_WATER_SPREAD_H
#define WORLD_FLUID_WATER_SPREAD_H

#include "fluid_cell.h"

// water spread pass. flows down first, then equalises sideways. operates
// purely on the grid, double-buffered so order doesnt bias the result.

// how far (in cells) water will still spread sideways from a source before
// it gives up. real mc uses 7.
#define VOXL_FLUID_WATER_RANGE  7

// run one simulation step. returns the number of cells that changed, so the
// caller can stop ticking once the body has settled.
int voxl_fluid_water_step(voxl_fluid_grid *g);

// convenience: run up to max_steps or until settled. returns steps taken.
int voxl_fluid_water_settle(voxl_fluid_grid *g, int max_steps);

#endif
