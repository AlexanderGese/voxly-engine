#ifndef WORLD_GEN2_BIOME_TAIGA_H
#define WORLD_GEN2_BIOME_TAIGA_H

#include "gen2_types.h"

// taiga. cold conifer forest. rolling hills, snow dusting on the surface,
// podzol-ish dirt under the pines.

const gen2_biome_info *gen2_taiga_info(void);

int gen2_taiga_height(int wx, int wz, int sea_level, uint32_t seed);

block_id gen2_taiga_surface(const gen2_column *col, int y);

#endif
