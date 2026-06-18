#ifndef WORLD_GEN2_TYPES_H
#define WORLD_GEN2_TYPES_H
#include <stdint.h>
#include "../block.h"
// shared plain-data types for the gen2 biome experiments.
// nothing here touches chunks or render. its all numbers in, numbers out,
// so the worldgen driver can splice the result wherever it wants later.
// the climate inputs that pick a biome. all in [0,1].
typedef struct {
    float temperature;   // 0 cold .. 1 hot
    float humidity;      // 0 dry  .. 1 wet
    float continentality;// 0 ocean .. 1 deep inland
    float weirdness;     // extra noise for variant flips
} gen2_climate;
// our own biome ids, separate from the engine biome_id so we dont clash.
typedef enum {
    GEN2_MESA = 0,
    GEN2_SWAMP,
    GEN2_TAIGA,
    GEN2_SAVANNA,
    GEN2_JUNGLE,
    GEN2_BIOME_COUNT
} gen2_biome_id;
typedef struct {
    const char *name;
    block_id surface;     // very top block
    block_id subsurface;  // few blocks under surface
    block_id filler;      // deep filler (usually stone)
    float    base_height; // added to sea level
    float    height_amp;  // vertical variation
    float    tree_density;// trees per column probability
    float    grass_density;
    int      snowy;       // 1 if snow can settle on top
} gen2_biome_info;
typedef struct {
    int   wx, wz;        // world column coords
    int   sea_level;     // engine sea level, passed in
    int   height;        // terrain top y for this column
    uint32_t seed;
    gen2_climate climate;
    gen2_biome_id biome;
} gen2_column;
typedef block_id (*gen2_surface_fn)(const gen2_column *col, int y);
#endif
