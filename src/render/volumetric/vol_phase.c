#include "vol_phase.h"
#include "vol_config.h"

#include <math.h>

// cpu side of the scattering math. the glsl in volumetric_march.frag mirrors
// this line for line; if you change one, change the other or the tests drift.

static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

void volumetric_medium_init(volumetric_medium *m,
                            float g, float scatter, float extinct) {
    // g lives in (-VOL_G_MAX, VOL_G_MAX). the hg denominator goes singular as
    // |g| -> 1 so we never let it actually touch the edge.
    m->g = clampf(g, -VOL_G_MAX, VOL_G_MAX);

    // negative coefficients are nonsense; clamp to zero.
    m->scatter = scatter < 0.0f ? 0.0f : scatter;
    m->extinct = extinct < 0.0f ? 0.0f : extinct;

    // extinction must dominate scattering or every bounce nets energy and the
    // image marches off to white. nudge it up if the caller got it backwards.
    if (m->extinct < m->scatter) m->extinct = m->scatter;
}

int volumetric_medium_valid(const volumetric_medium *m) {
    if (!(fabsf(m->g) < 1.0f)) return 0;
    if (m->scatter < 0.0f || m->extinct < 0.0f) return 0;
    if (m->extinct < m->scatter) return 0;
    return 1;
}

float volumetric_phase_hg(float g, float cos_theta) {
    // henyey-greenstein. normalized so the integral over the sphere is 1.
    // denom = (1 + g^2 - 2 g cos)^(3/2). at g=0 this collapses to 1/4pi.
    float g2 = g * g;
    float denom = 1.0f + g2 - 2.0f * g * cos_theta;
    if (denom < 1e-6f) denom = 1e-6f;            // paranoia against fp slop
    denom = denom * sqrtf(denom);                 // ^1.5
    const float inv4pi = 0.0795774715f;           // 1/(4*pi)
    return inv4pi * (1.0f - g2) / denom;
}

float volumetric_transmittance(float extinct, float dist) {
    if (dist <= 0.0f) return 1.0f;
    return expf(-extinct * dist);
}

float volumetric_cos_between(vec3 a, vec3 b) {
    // assumes both already normalized; clamp guards acos-adjacent callers.
    return clampf(vec3_dot(a, b), -1.0f, 1.0f);
}
