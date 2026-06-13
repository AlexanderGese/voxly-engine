#ifndef WORLD_BIOME_COLUMN_H
#define WORLD_BIOME_COLUMN_H

#include <stdint.h>
#include "biome_def.h"
#include "biome_climate.h"

// the resolved per-column context. building one runs the whole pipeline:
// sample climate, altitude-correct, pick biome, blend height. the surface
// rules then read this and report a block id per y. pure data, no chunk refs.

typedef struct {
    int          wx, wz;       // world column coords
    int          sea_level;    // engine WORLD_SEA_LEVEL, passed in
    uint32_t     seed;
    int          height;       // blended terrain top y
    biome_climate climate;     // altitude-corrected climate
    biome_kind   biome;        // dominant biome after blend
    float        sharpness;    // border proximity 0..1
} biome_column;

// run the full pipeline for a column. radius drives the height blend kernel.
void biome_column_build(biome_column *col, int wx, int wz,
                        int sea_level, uint32_t seed, int radius);

#endif
