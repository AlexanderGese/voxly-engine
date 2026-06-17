#ifndef WORLD_GEN2_BIOME_SWAMP_H
#define WORLD_GEN2_BIOME_SWAMP_H

#include "gen2_types.h"

// swamp. flat, low, sits right around sea level so water pools everywhere.
// muddy surface, occasional shallow pools.

const gen2_biome_info *gen2_swamp_info(void);

int gen2_swamp_height(int wx, int wz, int sea_level, uint32_t seed);

block_id gen2_swamp_surface(const gen2_column *col, int y);

// true if this column should be a water pool (top below sea level).
int gen2_swamp_is_pool(int wx, int wz, int sea_level, uint32_t seed);

#endif
