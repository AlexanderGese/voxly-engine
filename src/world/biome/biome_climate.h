#ifndef WORLD_BIOME_CLIMATE_H
#define WORLD_BIOME_CLIMATE_H
#include <stdint.h>
typedef struct {
    float temperature;    // 0 frozen .. 1 scorching
    float humidity;       // 0 arid   .. 1 soaked
    float erosion;        // 0 jagged highlands .. 1 worn-flat lowlands
    float weirdness;      // signed-ish extra field for rare variant flips
} biome_climate;
float biome_climate_temperature(int wx, int wz, uint32_t seed);
float biome_climate_humidity(int wx, int wz, uint32_t seed);
float biome_climate_erosion(int wx, int wz, uint32_t seed);
#endif
