#ifndef WORLD_WEATHERSIM_ACCUM_H
#define WORLD_WEATHERSIM_ACCUM_H

#include "weathersim_types.h"
#include "weathersim_field.h"

// accumulation -> ground state. the precip pass banks an abstract "accum" float
// per cell; this turns it into something the world decorator can actually act
// on: a snow layer depth in blocks, a wetness 0..1 for darkening the ground
// texture, and a frozen flag for ponds/edges. it's a read-only view over the
// field — it never mutates the sim, just interprets it.
//
// the mapping is deliberately chunky: accum is continuous but a voxel world
// wants integer snow layers, so we quantise with a little hysteresis baked into
// the thresholds (snow appears at 1.0, needs to drop under 0.6 to vanish a
// layer) so a column on the edge doesn't flicker its snow on and off each tick.

typedef struct {
    int   snow_layers;   // 0..WEATHERSIM_SNOW_MAX, integer block layers
    float wetness;       // 0..1, surface darkening / puddle alpha
    int   frozen;        // 1 if standing water here should be ice
    float accum;         // raw accumulation passed through, for debugging
} weathersim_ground;

// max snow layers we'll ever stack. minecraft uses 8 sub-layers; we cap lower
// because voxl's snow block is whole-voxel and tall drifts look silly.
#define WEATHERSIM_SNOW_MAX 4

// interpret the cell containing a world column. altitude matters for the freeze
// decision the same way it does for precip type.
weathersim_ground weathersim_accum_ground(const weathersim_field *f,
                                          const weathersim_params *params,
                                          int gx, int gz, float column_y);

// convenience: snow layer count for a world position, the one number the chunk
// decorator usually wants. returns 0 where it's too warm or too dry.
int weathersim_accum_snow_layers(const weathersim_field *f,
                                 const weathersim_params *params,
                                 vec3 world_pos);

#endif
