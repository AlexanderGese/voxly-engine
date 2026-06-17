#ifndef WORLD_GEN2_COLUMN_FILL_H
#define WORLD_GEN2_COLUMN_FILL_H

#include <stdint.h>
#include "../block.h"
#include "gen2_types.h"

// ties the whole gen2 stack together for one vertical column. given world
// coords it builds the column (climate->biome->height), then fills a caller
// supplied array of block ids from y=0 up. applies cave carving and water.
// totally self-contained: numbers in, a stack of block ids out.

// fill out[0..max_y-1] with block ids for the column at (wx,wz).
// returns the surface height used. col is populated as a side output if
// non-null (handy for then placing trees/deco on top).
int gen2_column_fill(block_id *out, int max_y, int wx, int wz,
                     int sea_level, uint32_t seed, gen2_column *col_out);

// count solid (non-air, non-water) blocks in a filled column. test helper.
int gen2_column_solid_count(const block_id *col, int max_y);

#endif
