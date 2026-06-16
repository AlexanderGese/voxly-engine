#ifndef WORLD_FLUID_BUOYANCY_H
#define WORLD_FLUID_BUOYANCY_H

#include "fluid_cell.h"
#include <stdbool.h>

// archimedes-ish buoyancy for things floating/sinking in a fluid. we keep our
// own tiny body struct so this doesnt depend on the player module at all; the
// game glue copies values in and out.

typedef struct {
    float density;       // kg per "block", relative. water = 1.0
    float volume;        // submerged volume tracked here for convenience
    float vy;            // vertical velocity (only axis that matters for float)
} voxl_fluid_body;

// reference fluid densities. lava is heavier so things float higher in it.
#define VOXL_FLUID_WATER_DENSITY  1.0f
#define VOXL_FLUID_LAVA_DENSITY   3.0f

// density of the fluid a cell holds, 0 if none.
float voxl_fluid_density_of(const voxl_fluid_cell *c);

// fraction 0..1 of a body submerged given its bottom y and height, and the
// fluid surface height in the same units.
float voxl_fluid_submerged_fraction(float body_bottom, float body_height,
                                     float surface_y);

// net vertical acceleration from buoyancy + gravity for a submerged body.
// positive = upward. gravity should be passed as a positive magnitude.
float voxl_fluid_buoyant_accel(const voxl_fluid_body *b, float fluid_density,
                               float submerged, float gravity);

// integrate one step: applies buoyancy, gravity and drag to vy. returns the
// new vy (also written back into b).
float voxl_fluid_body_integrate(voxl_fluid_body *b, float fluid_density,
                                float submerged, float gravity, float dt);

#endif
