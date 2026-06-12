#ifndef WORLD_BLOCK_H
#define WORLD_BLOCK_H

#include <stdint.h>

typedef uint8_t block_id;

enum {
    BLOCK_AIR = 0,
    BLOCK_STONE,
    BLOCK_DIRT,
    BLOCK_GRASS,
    BLOCK_SAND,
    BLOCK_WOOD,
    BLOCK_LEAVES,
    BLOCK_PLANKS,
    BLOCK_COBBLE,
    BLOCK_BEDROCK,
    BLOCK_GLASS,
    BLOCK_WATER,
    BLOCK_TORCH,
    BLOCK_BRICK,
    BLOCK_SNOW,
    BLOCK_ICE,
    BLOCK_CACTUS,
    BLOCK_COUNT
};

typedef struct {
    const char *name;
    int    solid;          // collides with player
    int    opaque;         // blocks light and occludes faces
    int    emits_light;    // emits light
    int    luminance;      // 0..15
    // atlas tile per face: 0=top, 1=bottom, 2=side
    int    tile_top;
    int    tile_bot;
    int    tile_side;
} block_info;

const block_info *block_get(block_id id);

// shorthands used all over the place
int  block_is_solid(block_id id);
int  block_is_opaque(block_id id);
int  block_is_air(block_id id);
int  block_is_transparent(block_id id);

// returns the tile id to use for a given face of a block.
// face 0=+x, 1=-x, 2=+y(top), 3=-y(bot), 4=+z, 5=-z
int  block_face_tile(block_id id, int face);

#endif
