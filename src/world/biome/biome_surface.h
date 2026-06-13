#ifndef WORLD_BIOME_SURFACE_H
#define WORLD_BIOME_SURFACE_H

#include "biome_def.h"
#include "biome_column.h"
#include "../block.h"

// surface rules. given a resolved column and a y, report which block goes
// there. this is the material stack: bedrock floor, deep stone filler, a band
// of subsurface, the top block, then water up to sea level, then air. snow
// caps and underwater swaps are folded in here.

block_id biome_surface_block(const biome_column *col, int y);

// the very top exposed block for a column (what you'd see from above). useful
// for minimaps and deco placement without walking the whole stack.
block_id biome_surface_top(const biome_column *col);

// fill a vertical strip [y0,y1) into out[] in one shot. returns how many were
// written (y1-y0 clamped). saves a per-y dispatch for the chunk filler.
int biome_surface_fill(const biome_column *col, int y0, int y1, block_id *out);

#endif
