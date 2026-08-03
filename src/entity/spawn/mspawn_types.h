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
#define MSPAWN_PACK_MAX 8
#endif
