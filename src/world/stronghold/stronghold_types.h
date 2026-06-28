#ifndef WORLD_STRONGHOLD_TYPES_H
#define WORLD_STRONGHOLD_TYPES_H

#include <stdint.h>
#include "../block.h"

// plain-data types for the stronghold generator. like structgen this stays
// chunk-free and render-free: we grow a room graph in abstract space, carve
// it, and emit voxels into a buffer. the worldgen driver stamps them wherever.
//
// a stronghold is a buried maze of stone-brick rooms joined by corridors,
// with exactly one portal room and (usually) one or two library rooms. the
// generator is a guided random walk over a room graph, same spirit as the
// dungeon assembler but bigger, deeper, and with typed rooms.

// cardinal facing. -z/+x/+z/-x. kept as ints so we rotate by adding mod 4,
// matching structgen_dir so callers can splice the two without translating.
typedef enum {
    STRONGHOLD_NORTH = 0,   // -z
    STRONGHOLD_EAST  = 1,   // +x
    STRONGHOLD_SOUTH = 2,   // +z
    STRONGHOLD_WEST  = 3,   // -x
    STRONGHOLD_DIR_COUNT
} stronghold_dir;

// what a room is. drives the carver and the decorator.
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

// a single placed voxel in world space. mirrors structgen_voxel on purpose.
typedef struct {
    int x, y, z;
    block_id id;
} stronghold_voxel;

// integer box, inclusive min, exclusive max. same convention as structgen_box.
typedef struct {
    int x0, y0, z0;   // min corner
    int x1, y1, z1;   // max corner (exclusive)
} stronghold_box;

// a doorway punched in a wall between two rooms. kind decides how it looks:
// open arch, plank door, or iron bars (prison). stored on the edge so the
// carver can render it after both rooms exist.
typedef enum {
    STRONGHOLD_DOOR_ARCH = 0,    // open 1x2 hole
    STRONGHOLD_DOOR_PLANK,       // wood door block in the hole
    STRONGHOLD_DOOR_BARS,        // iron bars (we fake with glass), prison cells
    STRONGHOLD_DOOR_GRATE        // half-height crawl gap
} stronghold_door_kind;

// a doorway cell recorded by the corridor carver and dressed later by the door
// decorator. lives on the room it pierces. `gated` is set when the corridor
// touches a prison/portal room, forcing iron bars regardless of the roll.
typedef struct {
    int x, y, z;
    stronghold_dir facing;   // dir pointing out of the room through this hole
    int gated;               // 1 = force bars
} stronghold_door;

// a stronghold occupies a region rooted at an anchor world position. ground_y
// is filled by the driver; the maze buries itself a fixed depth below it.
typedef struct {
    int anchor_x, anchor_z;   // world xz the stronghold is rooted at
    int ground_y;             // surface y under the anchor (driver fills)
    uint32_t seed;            // derived per-stronghold seed
} stronghold_site;

// tunables. the driver owns one of these.
typedef struct {
    int   sea_level;
    int   min_depth;          // how far below ground the top level sits
    int   level_height;       // vertical spacing between stacked levels
    int   max_rooms;          // hard cap on rooms in the graph
    int   max_levels;         // how many stacked floors we allow
    float library_chance;     // per eligible room, [0,1]
    float prison_chance;      // per eligible room, [0,1]
    int   max_voxels;         // hard cap on a single stronghold
} stronghold_config;

stronghold_config stronghold_config_default(void);

// human-readable room name, for logs/debug. never NULL.
const char *stronghold_room_type_name(stronghold_room_type t);

#endif
