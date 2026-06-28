#include "stronghold_types.h"

stronghold_config stronghold_config_default(void) {
    stronghold_config c;
    c.sea_level      = 64;
    c.min_depth      = 22;    // top of the maze sits well under the dirt
    c.level_height   = 9;     // 5-high rooms + 1 floor/ceiling + slack
    c.max_rooms      = 48;
    c.max_levels     = 3;
    c.library_chance = 0.22f;
    c.prison_chance  = 0.18f;
    c.max_voxels     = 200000;
    return c;
}

const char *stronghold_room_type_name(stronghold_room_type t) {
    switch (t) {
        case STRONGHOLD_ROOM_HALL:     return "hall";
        case STRONGHOLD_ROOM_JUNCTION: return "junction";
        case STRONGHOLD_ROOM_PRISON:   return "prison";
        case STRONGHOLD_ROOM_LIBRARY:  return "library";
        case STRONGHOLD_ROOM_PORTAL:   return "portal";
        case STRONGHOLD_ROOM_STAIRWELL:return "stairwell";
        default:                       return "none";
    }
}
