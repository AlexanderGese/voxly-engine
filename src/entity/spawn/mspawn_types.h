#ifndef ENTITY_SPAWN_MSPAWN_TYPES_H
#define ENTITY_SPAWN_MSPAWN_TYPES_H
#include <stdint.h>
#include "../../math/vec3.h"
#include "../../world/biome.h"
#include "../entity.h"
typedef enum {
    MSPAWN_TIME_ANY = 0,   // doesnt care about the clock
    MSPAWN_TIME_DAY,       // sun is up
    MSPAWN_TIME_NIGHT,     // sun is down, the dangerous window
    MSPAWN_TIME_DUSK,      // narrow band around sunset/sunrise
    MSPAWN_TIME_COUNT
} mspawn_time_band;
typedef enum {
    MSPAWN_CAT_PASSIVE = 0,
    MSPAWN_CAT_HOSTILE,
    MSPAWN_CAT_AMBIENT,
    MSPAWN_CAT_COUNT
} mspawn_category;
typedef enum {
    MSPAWN_GROUND_SOLID = 0, // top of an opaque solid block, open air above
    MSPAWN_GROUND_WATER,     // submerged, in a water column
    MSPAWN_GROUND_AIR        // anywhere with headroom, cave dwellers
} mspawn_ground_kind;
typedef struct {
    entity_type        type;
    mspawn_category    category;
    mspawn_time_band   time_band;
    mspawn_ground_kind ground;
    int                min_light;     // lowest block+sun light it tolerates
    int                max_light;     // highest light it tolerates (hostiles low)
    int                y_min, y_max;   // inclusive world-y band
    int                pack_min;       // floor on pack size when chosen
    int                pack_max;       // ceiling on pack size
    float              weight;         // relative pick weight within a biome
} mspawn_entry;
#define MSPAWN_PACK_MAX 8
#endif
