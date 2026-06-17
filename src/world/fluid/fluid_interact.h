#ifndef WORLD_FLUID_FLUID_INTERACT_H
#define WORLD_FLUID_FLUID_INTERACT_H

#include "fluid_cell.h"

// fluid-vs-fluid and fluid-vs-environment interactions that run after the
// spread passes: lava + water -> rock, freezing, evaporation. these mutate
// the grid (and report converted cells) but leave actually swapping blocks in
// the real world to the caller.

// result of a single interaction sweep.
typedef struct {
    int hardened;    // lava cells turned solid
    int frozen;      // water cells turned solid (ice)
    int evaporated;  // shallow water cells removed
} voxl_fluid_interact_result;

// where lava meets water, lava hardens to solid and the touching water is
// consumed. flowing lava -> cobble, source lava -> obsidian (caller decides
// the block id from the returned solid mask). returns counts.
voxl_fluid_interact_result voxl_fluid_resolve_contact(voxl_fluid_grid *g);

// freeze exposed water cells when `cold` is set (e.g. snowy biome at night).
// only freezes still (level==FULL, non-falling) surface water.
int voxl_fluid_freeze_pass(voxl_fluid_grid *g, bool cold);

// evaporate single-level water with nothing above and air all around.
int voxl_fluid_evaporate_pass(voxl_fluid_grid *g);

#endif
