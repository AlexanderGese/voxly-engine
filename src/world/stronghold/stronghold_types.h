#ifndef WORLD_STRONGHOLD_TYPES_H
#define WORLD_STRONGHOLD_TYPES_H
#include <stdint.h>
#include "../block.h"
typedef enum {
    STRONGHOLD_NORTH = 0,   // -z
    STRONGHOLD_EAST  = 1,   // +x
    STRONGHOLD_SOUTH = 2,   // +z
    STRONGHOLD_WEST  = 3,   // -x
    STRONGHOLD_DIR_COUNT
} stronghold_dir;
typedef enum {
    STRONGHOLD_ROOM_NONE = 0,
    STRONGHOLD_ROOM_HALL,        // plain rectangular hall, the connective tissue
    STRONGHOLD_ROOM_JUNCTION,    // small 4-way hub, lots of doors
    STRONGHOLD_ROOM_PRISON,      // hall lined with iron-bar cells
    STRONGHOLD_ROOM_LIBRARY,     // bookshelves, walkways, the prize
    STRONGHOLD_ROOM_PORTAL,      // the portal room. exactly one per stronghold
    STRONGHOLD_ROOM_STAIRWELL,   // vertical shaft linking two levels
    STRONGHOLD_ROOM_TYPE_COUNT
} stronghold_room_type;
typedef struct {
    int x, y, z;
    block_id id;
} stronghold_voxel;
typedef struct {
    int x0, y0, z0;   // min corner
    int x1, y1, z1;   // max corner (exclusive)
} stronghold_box;
typedef enum {
    STRONGHOLD_DOOR_ARCH = 0,    // open 1x2 hole
    STRONGHOLD_DOOR_PLANK,       // wood door block in the hole
    STRONGHOLD_DOOR_BARS,        // iron bars (we fake with glass), prison cells
    STRONGHOLD_DOOR_GRATE        // half-height crawl gap
} stronghold_door_kind;
typedef struct {
    int x, y, z;
    stronghold_dir facing;   // dir pointing out of the room through this hole
    int gated;               // 1 = force bars
} stronghold_door;
#endif
