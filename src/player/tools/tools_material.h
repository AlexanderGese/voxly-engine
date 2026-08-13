#ifndef PLAYER_TOOLS_MATERIAL_H
#define PLAYER_TOOLS_MATERIAL_H

#include "tools_types.h"

// material definition: how hard a class of block is, what tool likes it, and
// the minimum harvest level required to get drops. hardness is in the same
// "seconds at speed 1" unit the vanilla-ish break time uses.

typedef struct {
    mat_class   cls;
    float       hardness;        // base seconds to break by hand at speed 1
    tool_kind   pref_tool;       // tool that gets the speed bonus
    int         required_level;  // harvest level needed for drops, 0 = anything
    int         tool_required;   // 1 = no drops without the right tool kind
} mat_def;

const mat_def *tools_material_def(mat_class cls);

float       tools_material_hardness(mat_class cls);
tool_kind   tools_material_pref(mat_class cls);
int         tools_material_required_level(mat_class cls);

// is `kind` the effective tool for this material? hand is never effective
// except on MAT_NONE.
int         tools_material_effective(mat_class cls, tool_kind kind);

#endif
