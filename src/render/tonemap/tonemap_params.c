#include "tonemap_params.h"

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void tonemap_params_default(tonemap_params *p) {
    p->curve_kind = TONEMAP_DEFAULT_CURVE;
    p->white      = TONEMAP_DEFAULT_WHITE;
    p->gamma      = TONEMAP_DEFAULT_GAMMA;

    tonemap_grade_init(&p->grade);

    p->lut_weight  = 0.0f;
    p->lut_enabled = 0;
    p->enabled     = 1;
}

void tonemap_params_sanitize(tonemap_params *p) {
    if (p->curve_kind < 0 || p->curve_kind >= TONEMAP_CURVE_COUNT)
        p->curve_kind = TONEMAP_DEFAULT_CURVE;

    // white point under 1 makes the extended-reinhard math fold the range; keep
    // it sensible. gamma of 0 would divide by zero in the shader.
    p->white = clampf(p->white, 1.0f, 64.0f);
    p->gamma = clampf(p->gamma, 1.0f, 4.0f);

    tonemap_grade_sanitize(&p->grade);

    p->lut_weight  = clampf(p->lut_weight, 0.0f, 1.0f);
    p->lut_enabled = p->lut_enabled ? 1 : 0;
    p->enabled     = p->enabled ? 1 : 0;
}

tonemap_curve tonemap_params_make_curve(const tonemap_params *p, float exposure) {
    tonemap_curve c;
    tonemap_curve_init(&c, p->curve_kind);
    c.white = p->white;
    tonemap_curve_set_exposure(&c, exposure);
    return c;
}
