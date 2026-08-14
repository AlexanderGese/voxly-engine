#ifndef PLAYER_TOOLS_TYPES_H
#define PLAYER_TOOLS_TYPES_H
#include <stdint.h>
typedef enum {
    TOOL_HAND = 0,
    TOOL_PICKAXE,
    TOOL_AXE,
    TOOL_SHOVEL,
    TOOL_HOE,
    TOOL_SHEARS,
    TOOL_SWORD,        // mostly for combat but it digs cobwebs/leaves fast
    TOOL_KIND_COUNT
} tool_kind;
typedef enum {
    TIER_WOOD = 0,
    TIER_STONE,
    TIER_IRON,
    TIER_GOLD,         // fast but fragile, sits oddly between wood and iron level-wise
    TIER_DIAMOND,
    TIER_COUNT
} tool_tier;
typedef enum {
    MAT_NONE = 0,      // air / instant
    MAT_STONE,
    MAT_DIRT,
    MAT_WOOD,
    MAT_LEAVES,
    MAT_SAND,
    MAT_GLASS,
    MAT_METAL,
    MAT_GEM,           // obsidian, diamond block etc, brutally hard
    MAT_PLANT,
    MAT_CLOTH,
    MAT_LIQUID,
    MAT_CLASS_COUNT
} mat_class;
typedef enum {
    MINE_IDLE = 0,     // nothing happening
    MINE_PROGRESS,     // chipping away
    MINE_BROKE,        // block came loose this tick
    MINE_BLOCKED,      // tool can't harvest at all (instant-mine impossible)
    MINE_TOOL_SPENT    // tool broke before the block did
} mine_result;
#endif
