#ifndef RENDER_VOLUMETRIC_PARAMS_H
#define RENDER_VOLUMETRIC_PARAMS_H

#include "vol_phase.h"

// the runtime-mutable knobs, separated from the pass so the debug ui (and the
// tests) can poke them without dragging in gl. all setters clamp to the sane
// ranges from vol_config.h; the getters never return garbage.

typedef struct {
    int    steps;        // march samples per ray
    int    scale;        // resolution downsample factor
    float  g;            // hg anisotropy
    float  scatter;      // scattering coeff
    float  extinct;      // extinction coeff
    float  intensity;    // composite multiplier
    float  max_dist;     // march cutoff, world units
    int    enabled;      // master on/off (separate from "shaders loaded")
} volumetric_params;

// fill with the config defaults.
void volumetric_params_defaults(volumetric_params *p);

// clamped setters. each keeps the value in its documented range and, for the
// medium coefficients, preserves extinct >= scatter.
void volumetric_params_set_steps(volumetric_params *p, int steps);
void volumetric_params_set_g(volumetric_params *p, float g);
void volumetric_params_set_scatter(volumetric_params *p, float s);
void volumetric_params_set_extinct(volumetric_params *p, float e);
void volumetric_params_set_intensity(volumetric_params *p, float i);
void volumetric_params_set_max_dist(volumetric_params *p, float d);

// bake the medium-related params into a validated volumetric_medium.
void volumetric_params_to_medium(const volumetric_params *p,
                                 volumetric_medium *out);

#endif
