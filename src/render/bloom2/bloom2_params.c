#include "bloom2_params.h"

void bloom2_params_default(bloom2_params *p) {
    p->threshold = BLOOM2_DEFAULT_THRESHOLD;
    p->knee      = BLOOM2_DEFAULT_KNEE;
    p->intensity = BLOOM2_DEFAULT_INTENSITY;
    p->radius    = BLOOM2_DEFAULT_RADIUS;
    p->clamp_max = BLOOM2_FIREFLY_CLAMP;
    p->mip_count = BLOOM2_MAX_MIPS;
    p->enabled   = 1;
}

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void bloom2_params_sanitize(bloom2_params *p) {
    p->threshold = clampf(p->threshold, 0.0f, 64.0f);
    p->knee      = clampf(p->knee,      0.0f, 8.0f);
    p->intensity = clampf(p->intensity, 0.0f, 4.0f);
    p->radius    = clampf(p->radius,    0.0f, 8.0f);

    // clamp_max below the threshold would clip everything to black, which is
    // not what anyone wants. keep it at least a hair above threshold.
    if (p->clamp_max < p->threshold + 1e-3f)
        p->clamp_max = p->threshold + 1e-3f;

    if (p->mip_count < 1) p->mip_count = 1;
    if (p->mip_count > BLOOM2_MAX_MIPS) p->mip_count = BLOOM2_MAX_MIPS;

    p->enabled = p->enabled ? 1 : 0;
}

void bloom2_params_knee_curve(const bloom2_params *p, float out[4]) {
    float knee = p->knee;
    out[0] = p->threshold;
    out[1] = p->threshold - knee;
    out[2] = 2.0f * knee;
    out[3] = 0.25f / (knee + 1e-5f);
}
