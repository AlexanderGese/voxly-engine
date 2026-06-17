#ifndef WORLD_GEN2_CLIMATE_H
#define WORLD_GEN2_CLIMATE_H

#include "gen2_types.h"

// climate maps. temperature/humidity/continentality sampled from value
// noise at world coords. these drive biome selection in biome_registry.

// fill a gen2_climate for a world column. deterministic on (wx,wz,seed).
void gen2_climate_sample(int wx, int wz, uint32_t seed, gen2_climate *out);

// temperature gets a latitude-ish falloff so poles are cold. wz drives it.
float gen2_climate_temperature(int wx, int wz, uint32_t seed);
float gen2_climate_humidity(int wx, int wz, uint32_t seed);
float gen2_climate_continentality(int wx, int wz, uint32_t seed);

#endif
