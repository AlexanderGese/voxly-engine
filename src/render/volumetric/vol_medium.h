#ifndef RENDER_VOLUMETRIC_MEDIUM_H
#define RENDER_VOLUMETRIC_MEDIUM_H
#include "vol_phase.h"
#include "../../math/vec3.h"
typedef struct {
    volumetric_medium medium;   // validated scattering params
    vec3              tint;     // per-channel multiplier, ~white at noon
    float             strength; // 0 at night, ramps up near the horizon
} volumetric_medium_profile;
void volumetric_medium_profile_from_sun(volumetric_medium_profile *p,
                                        vec3 to_sun,
                                        float base_g,
                                        float base_scatter,
                                        float base_extinct);
float volumetric_sun_elevation(vec3 to_sun);
#endif
