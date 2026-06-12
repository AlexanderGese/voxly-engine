#ifndef WORLD_BIOME_H
#define WORLD_BIOME_H

// biomes are chosen from 2d noise sampled at world coords.
// we pick from temperature and humidity.

#include "block.h"

typedef enum {
    BIOME_PLAINS = 0,
    BIOME_FOREST,
    BIOME_DESERT,
    BIOME_MOUNTAINS,
    BIOME_TUNDRA,
    BIOME_SWAMP,
    BIOME_BEACH,
    BIOME_COUNT
} biome_id;

typedef struct {
    const char *name;
    block_id    surface;
    block_id    filler;
    block_id    top_deco;  // BLOCK_AIR if none
    float       tree_density;
    float       height_bias;
    float       height_amp;
    int         has_snow;
} biome_info;

const biome_info *biome_get(biome_id id);
biome_id  biome_pick(float temperature, float humidity);

// sample temperature/humidity for a world coord
void biome_sample(int wx, int wz, unsigned seed, float *t, float *h);

#endif
