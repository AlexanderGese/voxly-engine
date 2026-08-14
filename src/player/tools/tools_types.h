#ifndef PLAYER_TOOLS_TYPES_H
#define PLAYER_TOOLS_TYPES_H

#include <stdint.h>

// shared vocabulary for the tools+mining subsystem. kept in one header so
// the rest of the module doesn't end up with a tangle of cross includes.

// what kind of tool an item is. hand is the implicit "no tool" case and is
// never an actual item, it's just the fallback when nothing useful is held.
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

// material tier of a tool head. ordered, higher == better. the numbers double
// as a harvest-level so we can compare against a block's required level.
typedef enum {
    TIER_WOOD = 0,
    TIER_STONE,
    TIER_IRON,
    TIER_GOLD,         // fast but fragile, sits oddly between wood and iron level-wise
    TIER_DIAMOND,
    TIER_COUNT
} tool_tier;

// broad material class of a block. drives which tool is "effective" and the
// base hardness. ripped roughly from how the worldgen groups blocks.
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

// result codes for a finished mining tick. callers switch on these.
typedef enum {
    MINE_IDLE = 0,     // nothing happening
    MINE_PROGRESS,     // chipping away
    MINE_BROKE,        // block came loose this tick
    MINE_BLOCKED,      // tool can't harvest at all (instant-mine impossible)
    MINE_TOOL_SPENT    // tool broke before the block did
} mine_result;

// a tool head has up to one preferred kind and a tier. -1 tier means "this is
// the hand", which has no durability and a flat dig speed of 1.
typedef struct {
    tool_kind kind;
    tool_tier tier;
} tool_head;

#endif
