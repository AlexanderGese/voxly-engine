#ifndef PLAYER_FISHING_TYPES_H
#define PLAYER_FISHING_TYPES_H
#include <stdint.h>
#include "../../math/vec3.h"
#include "../../world/block.h"
typedef enum {
    WATER_NONE = 0,    // no water at all, line just lies there
    WATER_OPEN,        // a tidy patch of water, the good case
    WATER_OBSTRUCTED,  // water but boxed in by blocks, slower bites
    WATER_KIND_COUNT
} fishing_water_kind;
#endif
