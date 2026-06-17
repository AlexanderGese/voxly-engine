#ifndef WORLD_FLUID_VISCOSITY_H
#define WORLD_FLUID_VISCOSITY_H

#include "fluid_cell.h"

// viscosity controls how fast a fluid responds and how much it drags things
// moving through it. water is thin, lava is thick/sludgy.

typedef struct {
    float drag;        // velocity multiplier per second moving through it
    int   flow_delay;  // ticks between spread updates
    float slope_bias;  // how strongly it prefers flowing downhill (0..1)
} voxl_fluid_viscosity;

// canned profiles for the known fluids.
voxl_fluid_viscosity voxl_fluid_viscosity_for(uint8_t kind);

// apply viscous drag to a velocity component over dt. returns new velocity.
float voxl_fluid_apply_drag(const voxl_fluid_viscosity *v, float vel, float dt);

// should a fluid of this viscosity update on the given tick?
bool voxl_fluid_should_flow(const voxl_fluid_viscosity *v, int tick);

// blend two viscosities (e.g. when a cell is partly mixed). t in 0..1.
voxl_fluid_viscosity voxl_fluid_viscosity_lerp(voxl_fluid_viscosity a,
                                               voxl_fluid_viscosity b, float t);

#endif
