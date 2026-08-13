#include "tools_material.h"
static const mat_def g_mats[MAT_CLASS_COUNT] = {
    // cls          hardness  pref_tool      req_lvl  tool_req
    { MAT_NONE,       0.0f,   TOOL_HAND,        0,      0 },
    { MAT_STONE,      1.5f,   TOOL_PICKAXE,     0,      1 },
    { MAT_DIRT,       0.5f,   TOOL_SHOVEL,      0,      0 },
    { MAT_WOOD,       2.0f,   TOOL_AXE,         0,      0 },
    { MAT_LEAVES,     0.2f,   TOOL_SHEARS,      0,      0 },
    { MAT_SAND,       0.5f,   TOOL_SHOVEL,      0,      0 },
    { MAT_GLASS,      0.3f,   TOOL_HAND,        0,      0 },
    { MAT_METAL,      3.0f,   TOOL_PICKAXE,     1,      1 },
    { MAT_GEM,       12.5f,   TOOL_PICKAXE,     3,      1 },  // obsidian-grade
    { MAT_PLANT,      0.0f,   TOOL_HAND,        0,      0 },  // instant pop
    { MAT_CLOTH,      0.8f,   TOOL_SHEARS,      0,      0 },
    { MAT_LIQUID,   100.0f,   TOOL_HAND,        0,      1 },  // effectively unbreakable
}
;
