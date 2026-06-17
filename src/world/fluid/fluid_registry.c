#include "fluid_registry.h"

#include <string.h>

// static table, indexed loosely by kind. order matches the VOXL_FLUID_* enum
// so a direct lookup is just an index when the kind is in range.
static const voxl_fluid_def voxl_fluid_defs[] = {
    {
        .name = "air", .kind = VOXL_FLUID_AIR,
        .spread_range = 0, .flow_delay = 1, .density = 0.0f,
        .tile = -1, .can_form_source = 0, .emits_light = 0, .luminance = 0,
    },
    {
        .name = "water", .kind = VOXL_FLUID_WATER,
        .spread_range = 7, .flow_delay = 1, .density = 1.0f,
        .tile = 32, .can_form_source = 1, .emits_light = 0, .luminance = 0,
    },
    {
        .name = "lava", .kind = VOXL_FLUID_LAVA,
        .spread_range = 3, .flow_delay = 4, .density = 3.0f,
        .tile = 48, .can_form_source = 0, .emits_light = 1, .luminance = 15,
    },
};

#define VOXL_FLUID_DEF_COUNT \
    ((int)(sizeof(voxl_fluid_defs) / sizeof(voxl_fluid_defs[0])))

const voxl_fluid_def *voxl_fluid_registry_get(uint8_t kind) {
    if (kind < VOXL_FLUID_DEF_COUNT && voxl_fluid_defs[kind].kind == kind)
        return &voxl_fluid_defs[kind];
    // fall back to a linear scan in case the table ever gets reordered
    for (int i = 0; i < VOXL_FLUID_DEF_COUNT; i++)
        if (voxl_fluid_defs[i].kind == kind) return &voxl_fluid_defs[i];
    return &voxl_fluid_defs[0];   // air
}

int voxl_fluid_registry_count(void) {
    return VOXL_FLUID_DEF_COUNT;
}

uint8_t voxl_fluid_registry_find(const char *name) {
    if (!name) return VOXL_FLUID_AIR;
    for (int i = 0; i < VOXL_FLUID_DEF_COUNT; i++)
        if (strcmp(voxl_fluid_defs[i].name, name) == 0)
            return voxl_fluid_defs[i].kind;
    return VOXL_FLUID_AIR;
}
