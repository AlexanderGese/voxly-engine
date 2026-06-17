#ifndef WORLD_GEN2_BIOME_JUNGLE_H
#define WORLD_GEN2_BIOME_JUNGLE_H

#include "gen2_types.h"

// jungle. hot, wet, lumpy terrain. dense canopy, thick undergrowth.
// the bumpiest of the bunch so the canopy has relief.

const gen2_biome_info *gen2_jungle_info(void);

int gen2_jungle_height(int wx, int wz, int sea_level, uint32_t seed);

block_id gen2_jungle_surface(const gen2_column *col, int y);

#endif
