#include "structgen_types.h"
#include "../../config.h"

structgen_config structgen_config_default(void) {
    structgen_config c;
    c.sea_level         = WORLD_SEA_LEVEL;
    c.region_size       = 8;     // a village hub roughly every 8 chunks
    c.village_per_region= 1;
    c.dungeon_chance    = 0.04f; // buried, so it can be fairly common
    c.ruin_chance       = 0.06f;
    c.max_voxels        = 8192;  // a big village still fits comfortably
    return c;
}
