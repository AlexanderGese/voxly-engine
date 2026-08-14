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
#endif
