#ifndef RENDER_VOLUMETRIC_PARAMS_H
#define RENDER_VOLUMETRIC_PARAMS_H
#include "vol_phase.h"
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
void volumetric_params_defaults(volumetric_params *p);
void volumetric_params_set_steps(volumetric_params *p, int steps);
void volumetric_params_set_g(volumetric_params *p, float g);
void volumetric_params_set_scatter(volumetric_params *p, float s);
void volumetric_params_set_extinct(volumetric_params *p, float e);
void volumetric_params_set_intensity(volumetric_params *p, float i);
void volumetric_params_set_max_dist(volumetric_params *p, float d);
void volumetric_params_to_medium(const volumetric_params *p,
                                 volumetric_medium *out);
#endif
