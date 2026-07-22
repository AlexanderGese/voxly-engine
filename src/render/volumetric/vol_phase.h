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
void  volumetric_medium_init(volumetric_medium *m,
                             float g, float scatter, float extinct);
int   volumetric_medium_valid(const volumetric_medium *m);
float volumetric_phase_hg(float g, float cos_theta);
float volumetric_transmittance(float extinct, float dist);
float volumetric_cos_between(vec3 a, vec3 b);
#endif
