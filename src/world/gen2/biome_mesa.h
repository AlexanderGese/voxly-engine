#ifndef WORLD_GEN2_BIOME_MESA_H
#define WORLD_GEN2_BIOME_MESA_H

#include "gen2_types.h"

// mesa / badlands. banded clay terraces, hard stepped cliffs.
// we fake colored clay with the blocks we actually have (sand/dirt/stone).

const gen2_biome_info *gen2_mesa_info(void);

// terrain top for a mesa column. lots of flat plateaus + sudden steps.
int gen2_mesa_height(int wx, int wz, int sea_level, uint32_t seed);

// surface block at y. terraces produce horizontal bands.
block_id gen2_mesa_surface(const gen2_column *col, int y);

// helper exposed for tests: which band index a y falls in.
int gen2_mesa_band(int y, int sea_level);

#endif
