#include "tools_material.h"

// indexed by mat_class. keep in sync with the enum.
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
};

const mat_def *tools_material_def(mat_class cls) {
    if (cls < 0 || cls >= MAT_CLASS_COUNT) return &g_mats[MAT_NONE];
    return &g_mats[cls];
}

float tools_material_hardness(mat_class cls) {
    return tools_material_def(cls)->hardness;
}

tool_kind tools_material_pref(mat_class cls) {
    return tools_material_def(cls)->pref_tool;
}

int tools_material_required_level(mat_class cls) {
    return tools_material_def(cls)->required_level;
}

int tools_material_effective(mat_class cls, tool_kind kind) {
    const mat_def *m = tools_material_def(cls);
    if (m->cls == MAT_NONE || m->cls == MAT_PLANT) return 1;  // anything works
    if (m->pref_tool == TOOL_HAND) return 1;  // no preference, hand is fine
    if (kind == m->pref_tool) return 1;
    // swords get a pass on cloth/leaves, they shred those.
    if (kind == TOOL_SWORD && (cls == MAT_CLOTH || cls == MAT_LEAVES)) return 1;
    return 0;
}
