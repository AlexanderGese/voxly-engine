#ifndef RENDER_VOLUMETRIC_PHASE_H
#define RENDER_VOLUMETRIC_PHASE_H

#include "../../math/vec3.h"

// the scattering math, on the cpu. the march shader does the exact same thing
// in glsl; keeping a c reference lets the raymarcher run headless for tests
// and documents what the magic numbers in the .frag actually mean.

// a little bundle of medium parameters passed around the marcher. these are
// the validated, clamped versions of the config defaults.
typedef struct {
    float g;            // henyey-greenstein anisotropy, clamped to VOL_G_MAX
    float scatter;      // scattering coefficient per world unit
    float extinct;      // extinction coefficient per world unit (>= scatter)
} volumetric_medium;

// fill a medium from raw (possibly silly) inputs, clamping g and forcing
// extinct >= scatter so the march can't manufacture light out of nothing.
void  volumetric_medium_init(volumetric_medium *m,
                             float g, float scatter, float extinct);

// returns 1 if the medium is energy-conserving-ish (extinct >= scatter and
// |g| < 1). the pass logs a warning if you hand it a bad one.
int   volumetric_medium_valid(const volumetric_medium *m);

// henyey-greenstein phase function. cos_theta is the cosine of the angle
// between the view ray direction and the direction *to* the light. returns
// the (normalized, integrates to 1 over the sphere) phase value.
float volumetric_phase_hg(float g, float cos_theta);

// beer-lambert transmittance over a path of `dist` world units through a
// medium with the given extinction. just exp(-extinct*dist), pulled out so
// the marcher and the tests agree on it.
float volumetric_transmittance(float extinct, float dist);

// cosine of the angle between two (assumed normalized) directions, clamped to
// [-1,1] so acos-adjacent callers don't trip on fp slop. handy glue.
float volumetric_cos_between(vec3 a, vec3 b);

#endif
