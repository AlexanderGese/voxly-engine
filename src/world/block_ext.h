#ifndef WORLD_BLOCK_EXT_H
#define WORLD_BLOCK_EXT_H
#include "block.h"
enum {
    BLOCK_IRON_ORE = BLOCK_COUNT,
    BLOCK_COAL_ORE,
    BLOCK_GOLD_ORE,
    BLOCK_DIAMOND_ORE,
    BLOCK_GRAVEL,
    BLOCK_CLAY,
    BLOCK_BOOKSHELF,
    BLOCK_MOSSY_COBBLE,
    BLOCK_OBSIDIAN,
    BLOCK_TNT,
    BLOCK_WOOL_WHITE,
    BLOCK_WOOL_RED,
    BLOCK_WOOL_BLUE,
    BLOCK_WOOL_GREEN,
    BLOCK_SLAB_STONE,
    BLOCK_SLAB_WOOD,
    BLOCK_FENCE,
    BLOCK_LADDER,
    BLOCK_DOOR_BOTTOM,
    BLOCK_DOOR_TOP,
    BLOCK_CRAFTING_TABLE,
    BLOCK_FURNACE,
    BLOCK_CHEST_BLOCK,
    BLOCK_FLOWER_RED,
    BLOCK_FLOWER_YELLOW,
    BLOCK_MUSHROOM_RED,
    BLOCK_MUSHROOM_BROWN,
    BLOCK_TALL_GRASS,
    // BLOCK_CACTUS now lives in the base block enum (block.h); kept there to
    // avoid a duplicate enumerator here.
    BLOCK_SUGARCANE,
    BLOCK_PUMPKIN,
    BLOCK_MELON,
    BLOCK_EXT_COUNT
}
typedef struct {
    const char *name;
    int    solid;
    int    opaque;
    int    emits_light;
    int    luminance;
    int    tile_top, tile_bot, tile_side;
    int    is_slab;
    int    is_plant;       // no collision, transparent, cross-mesh
    int    tool_required;  // 0=hand, 1=pickaxe, 2=axe, 3=shovel
    float  break_time;     // seconds to break
    int    drop_id;        // what block id drops (can differ from self)
    int    drop_count;
    int    stack_max;
} block_ext_info;
#endif
