#ifndef RENDER_VOLUMETRIC_MEDIUM_H
#define RENDER_VOLUMETRIC_MEDIUM_H

#include "vol_phase.h"
#include "../../math/vec3.h"

// time-of-day driven medium tuning. the air isn't the same at noon and at
// sunset: low sun means longer optical paths, redder light, fatter shafts.
// this maps a normalized sun elevation onto a validated volumetric_medium plus
// a tint colour the composite multiplies in.
//
// elevation convention: -1 = straight down (night), 0 = horizon, +1 = zenith.
// we derive it from the sun direction's y component, so callers just hand us
// the world-space "to sun" vector.

typedef struct {
    volumetric_medium medium;   // validated scattering params
    vec3              tint;     // per-channel multiplier, ~white at noon
    float             strength; // 0 at night, ramps up near the horizon
} volumetric_medium_profile;

// build a profile for a given "towards the sun" direction (need not be
// normalized — we normalize it). base_* are the config defaults the profile
// scales around.
void volumetric_medium_profile_from_sun(volumetric_medium_profile *p,
                                        vec3 to_sun,
                                        float base_g,
                                        float base_scatter,
                                        float base_extinct);

// elevation in [-1,1] from a (possibly unnormalized) to-sun vector.
float volumetric_sun_elevation(vec3 to_sun);

// the horizon-glow ramp: 0 when the sun is high or below ground, peaking just
// above the horizon where shafts are most dramatic. exposed for tests.
float volumetric_horizon_ramp(float elevation);

#endif
