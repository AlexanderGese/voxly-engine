#ifndef WORLD_GEN2_BIOME_REGISTRY_H
#define WORLD_GEN2_BIOME_REGISTRY_H

#include "gen2_types.h"

// central table + dispatch for the gen2 biomes. picks a biome from climate,
// then routes height/surface queries to the right module.

const gen2_biome_info *gen2_registry_info(gen2_biome_id id);

// pick a biome from climate inputs.
gen2_biome_id gen2_registry_pick(const gen2_climate *c);

// height + surface dispatch by biome id.
int gen2_registry_height(gen2_biome_id id, int wx, int wz,
                         int sea_level, uint32_t seed);
block_id gen2_registry_surface(const gen2_column *col, int y);

// build a fully-populated column for (wx,wz): samples climate, picks biome,
// computes height. ready to feed to gen2_registry_surface per y.
void gen2_registry_build_column(int wx, int wz, int sea_level,
                                uint32_t seed, gen2_column *out);

#endif
