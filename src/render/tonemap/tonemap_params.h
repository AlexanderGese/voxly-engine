#ifndef RENDER_TONEMAP_PARAMS_H
#define RENDER_TONEMAP_PARAMS_H

#include "tonemap_curve.h"
#include "tonemap_grade.h"
#include "tonemap_config.h"

// the full bag of knobs for the tonemap+grade pass, minus the exposure state
// machine (that lives in tonemap_exposure and ticks on its own clock). this is
// the thing the debug ui mutates and the pass reads each frame.

typedef struct {
    int   curve_kind;   // TONEMAP_CURVE_*
    float white;        // white point for the curves that use one
    float gamma;        // output encode gamma (we apply 1/gamma)

    tonemap_grade grade;

    float lut_weight;   // 0 = lut off, 1 = full lut. lerped on the gpu
    int   lut_enabled;  // gate, independent of weight (so 0-weight keeps tex)

    int   enabled;      // master switch; off => straight gamma-only passthrough
} tonemap_params;

// neutral defaults: aces curve, identity grade, lut off.
void tonemap_params_default(tonemap_params *p);

// clamp everything into range and sanitize the embedded grade.
void tonemap_params_sanitize(tonemap_params *p);

// build a tonemap_curve from the params + an externally-supplied exposure
// multiplier (from tonemap_exposure). keeps the curve module ignorant of
// exposure policy.
tonemap_curve tonemap_params_make_curve(const tonemap_params *p, float exposure);

#endif
