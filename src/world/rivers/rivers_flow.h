#ifndef WORLD_RIVERS_FLOW_H
#define WORLD_RIVERS_FLOW_H

#include "rivers_field.h"
#include "rivers_types.h"

// the flow pass. two steps that always run back to back:
//
// 1. direction  - for every cell pick the steepest downhill neighbour (d8)
// off the *filled* surface, so water never points into a pit
// the lake pass already drowned. cells with no lower neighbour
// get RIVERS_DIR_NONE.
// 2. accumulate - drop rain_per_cell (plus jitter) on every cell, then push
// it downstream following the directions. processing cells in
// descending height order means a cell's full upstream load is
// already deposited before we forward it. that's the standard
// topo-sort-by-height accumulation trick.
//
// the result lands in field->dir and field->accum and the rest of the pipeline
// reads them. nothing here allocates beyond an index array on the stack-ish.

// run direction assignment over the whole field. expects field->filled to hold
// the depression-filled surface (call the lake fill first). reads filled, writes
// dir.
void rivers_flow_directions(rivers_field *f, const rivers_params *p);

// run accumulation. expects dir to be populated. reads dir + filled, writes
// accum. returns the peak accumulation seen, handy for tuning the threshold.
float rivers_flow_accumulate(rivers_field *f, const rivers_params *p);

// convenience: directions then accumulate. most callers want this.
float rivers_flow_run(rivers_field *f, const rivers_params *p);

#endif
