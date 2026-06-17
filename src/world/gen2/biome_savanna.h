#ifndef WORLD_GEN2_BIOME_SAVANNA_H
#define WORLD_GEN2_BIOME_SAVANNA_H

#include "gen2_types.h"

// savanna. dry grassland, mostly flat with the odd low plateau. sparse
// acacia-ish trees, lots of tall grass.

const gen2_biome_info *gen2_savanna_info(void);

int gen2_savanna_height(int wx, int wz, int sea_level, uint32_t seed);

block_id gen2_savanna_surface(const gen2_column *col, int y);

#endif
