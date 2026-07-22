#include "vol_params.h"
#include "vol_config.h"

static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

void volumetric_params_defaults(volumetric_params *p) {
    p->steps     = VOL_STEPS_DEFAULT;
    p->scale     = VOL_DEFAULT_SCALE;
    p->g         = VOL_G_DEFAULT;
    p->scatter   = VOL_SCATTER_DEFAULT;
    p->extinct   = VOL_EXTINCT_DEFAULT;
    p->intensity = VOL_INTENSITY_DEFAULT;
    p->max_dist  = VOL_MAX_DISTANCE;
    p->enabled   = 1;
}

void volumetric_params_set_steps(volumetric_params *p, int steps) {
    p->steps = clampi(steps, 1, VOL_STEPS_MAX);
}

void volumetric_params_set_g(volumetric_params *p, float g) {
    p->g = clampf(g, -VOL_G_MAX, VOL_G_MAX);
}

void volumetric_params_set_scatter(volumetric_params *p, float s) {
    p->scatter = s < 0.0f ? 0.0f : s;
    // keep the ordering invariant so the pass never has to re-fix it.
    if (p->extinct < p->scatter) p->extinct = p->scatter;
}

void volumetric_params_set_extinct(volumetric_params *p, float e) {
    p->extinct = e < 0.0f ? 0.0f : e;
    if (p->extinct < p->scatter) p->extinct = p->scatter;
}

void volumetric_params_set_intensity(volumetric_params *p, float i) {
    // negative intensity would subtract light from the scene. no thanks.
    p->intensity = clampf(i, 0.0f, 16.0f);
}

void volumetric_params_set_max_dist(volumetric_params *p, float d) {
    // a metre of march minimum; anything past a few hundred is pointless once
    // transmittance has decayed, but we don't hard cap the top.
    p->max_dist = d < 1.0f ? 1.0f : d;
}

void volumetric_params_to_medium(const volumetric_params *p,
                                 volumetric_medium *out) {
    volumetric_medium_init(out, p->g, p->scatter, p->extinct);
}
