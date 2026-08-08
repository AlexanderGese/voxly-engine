#ifndef PLAYER_BUILDING_TYPES_H
#define PLAYER_BUILDING_TYPES_H

#include "../../math/vec3.h"
#include "../../world/block.h"

// shared vocabulary for the building module. raycast already gives us a
// ray_hit with a face index, but once you start placing/rotating blocks
// you need a bit more structure than three loose ints. so: this.

// face index convention matches player/raycast.h exactly. do not reorder,
// half the module indexes tables by this.
// 0=+x 1=-x 2=+y 3=-y 4=+z 5=-z
enum {
    BFACE_PX = 0,
    BFACE_NX,
    BFACE_PY,
    BFACE_NY,
    BFACE_PZ,
    BFACE_NZ,
    BFACE_COUNT
};

// horizontal facing for rotatable blocks (furnace, pumpkin, chest...).
// stored compactly so it can ride in a block's metadata nibble later.
enum {
    BFACING_NORTH = 0,   // -z
    BFACING_EAST,        // +x
    BFACING_SOUTH,       // +z
    BFACING_WEST,        // -x
    BFACING_COUNT
};

// axis for pillar-style blocks (wood logs). axis-aligned, no full facing.
enum {
    BAXIS_Y = 0,
    BAXIS_X,
    BAXIS_Z,
    BAXIS_COUNT
};

// result of a place attempt. negatives are failures, you can switch on these
// to drive a hud beep or whatever.
enum {
    BPLACE_OK            =  0,
    BPLACE_NO_TARGET     = -1,   // raycast missed
    BPLACE_OUT_OF_REACH  = -2,
    BPLACE_OCCUPIED      = -3,   // target cell isnt air/replaceable
    BPLACE_NO_SUPPORT    = -4,   // block needs something under/behind it
    BPLACE_PLAYER_OVERLAP= -5,   // would clip into the player
    BPLACE_EMPTY_HAND    = -6,   // nothing selected to place
    BPLACE_WORLD_EDGE    = -7,   // below y=0 or above build height
    BPLACE_NOT_PLACEABLE = -8    // block id refuses placement (bedrock etc)
};

// what a break produced. mostly so callers can spawn drops.
enum {
    BBREAK_OK           =  0,
    BBREAK_NO_TARGET    = -1,
    BBREAK_OUT_OF_REACH = -2,
    BBREAK_INDESTRUCT   = -3   // bedrock, you cant have it
};

// a fully resolved target: which block we hit and which cell a placement
// would land in (hit cell offset by the face normal).
typedef struct {
    int valid;
    int hit_x, hit_y, hit_z;     // the solid block we looked at
    int place_x, place_y, place_z; // adjacent cell, where a new block goes
    int face;                     // BFACE_* of the surface we hit
    float dist;                   // distance along the ray, blocks
} building_target;

// a single edit, recorded so we can undo. covers both place and break.
typedef struct {
    int      wx, wy, wz;
    block_id before;
    block_id after;
} building_edit;

#endif
