#ifndef WORLD_BIOME_CLIMATE_H
#define WORLD_BIOME_CLIMATE_H

#include <stdint.h>

// the climate field. four scalar params sampled from world coords. this is
// the input to the biome lookup. everything in [0,1] so the lookup can treat
// the param space as a unit hypercube and do nearest-fit without rescaling.

typedef struct {
    float temperature;    // 0 frozen .. 1 scorching
    float humidity;       // 0 arid   .. 1 soaked
    float erosion;        // 0 jagged highlands .. 1 worn-flat lowlands
    float weirdness;      // signed-ish extra field for rare variant flips
} biome_climate;

// sample each channel independently. handy when you only need one.
float biome_climate_temperature(int wx, int wz, uint32_t seed);
float biome_climate_humidity(int wx, int wz, uint32_t seed);
float biome_climate_erosion(int wx, int wz, uint32_t seed);
float biome_climate_weirdness(int wx, int wz, uint32_t seed);

// fill all four at once. cheaper than four calls if you need the whole vector.
void  biome_climate_sample(int wx, int wz, uint32_t seed, biome_climate *out);

// altitude correction: high columns read colder. call after sampling with the
// terrain top y so mountaintops can flip to snow biomes. modifies in place.
void  biome_climate_apply_altitude(biome_climate *c, int top_y, int sea_level);

#endif
