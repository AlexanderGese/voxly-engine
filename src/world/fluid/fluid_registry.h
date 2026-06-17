#ifndef WORLD_FLUID_FLUID_REGISTRY_H
#define WORLD_FLUID_FLUID_REGISTRY_H

#include "fluid_cell.h"
#include <stdint.h>

// a tiny registry of fluid types and their tunables, so the rest of the sim
// can ask "how far does this spread" / "what tile" without hardcoding. keeps
// water and lava (and any future fluids) described in one place.

typedef struct {
    const char *name;
    uint8_t kind;          // VOXL_FLUID_*
    int     spread_range;  // max sideways cells
    int     flow_delay;    // ticks between updates
    float   density;       // for buoyancy
    int     tile;          // atlas tile id for the still surface
    uint8_t can_form_source; // 1 = two sources spawn a third (water yes)
    uint8_t emits_light;   // lava glows
    int     luminance;     // 0..15
} voxl_fluid_def;

// look up a fluid by kind. never returns NULL; unknown kinds map to a safe
// "air" definition.
const voxl_fluid_def *voxl_fluid_registry_get(uint8_t kind);

// total number of registered fluids (including air).
int voxl_fluid_registry_count(void);

// find a fluid kind by name, returns VOXL_FLUID_AIR if not found.
uint8_t voxl_fluid_registry_find(const char *name);

#endif
