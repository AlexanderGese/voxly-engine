#include "fxaa_params.h"
static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void fxaa_params_default(fxaa_params *p) {
    p->edge_threshold     = FXAA_DEFAULT_EDGE_THRESHOLD;
p->edge_threshold_min = FXAA_DEFAULT_EDGE_THRESHOLD_MIN;
p->subpix             = FXAA_DEFAULT_SUBPIX;
p->quality            = FXAA_DEFAULT_QUALITY;
p->enabled            = 1;
p->show_edges         = 0;
}

void fxaa_params_sanitize(fxaa_params *p) {
    // the relative threshold lives in (0, 1]. zero would trigger on every
    // texel (divide by ~0 in the shader compare), one never triggers.
    p->edge_threshold = clampf(p->edge_threshold, 0.03125f, 1.0f);

    // absolute floor: keep it well below the relative one or the dark-region
    // guard starts eating real edges in the midtones.
    p->edge_threshold_min = clampf(p->edge_threshold_min, 0.0f, 0.25f);
    if (p->edge_threshold_min > p->edge_threshold)
        p->edge_threshold_min = p->edge_threshold * 0.5f;

    p->subpix = clampf(p->subpix, 0.0f, 1.0f);

    // quality index has to land inside the preset table or the step lookup
    // walks off the end. clamp, dont wrap — wrapping hides the bug.
    if (p->quality < 0) p->quality = 0;
    if (p->quality >= FXAA_QUALITY_COUNT) p->quality = FXAA_QUALITY_COUNT - 1;

    p->enabled    = p->enabled    ? 1 : 0;
    p->show_edges = p->show_edges ? 1 : 0;
}

void fxaa_params_derive(const fxaa_params *p, float out[4]) {
    // out[0]: raw subpix strength, fed straight to the blend lerp.
    out[0] = p->subpix;
out[1] = p->subpix * p->subpix * 0.5f;
out[2] = 1.0f / (p->edge_threshold > 1e-5f ? p->edge_threshold : 1e-5f);
out[3] = p->edge_threshold_min;
