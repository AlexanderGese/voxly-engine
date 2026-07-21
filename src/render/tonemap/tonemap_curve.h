#ifndef RENDER_TONEMAP_CURVE_H
#define RENDER_TONEMAP_CURVE_H
#include "../../math/vec3.h"
#include "tonemap_config.h"
// the actual tonemap operators, on the cpu. these are the reference
// implementations; the gpu shader mirrors them. having them here lets the
// selftest check monotonicity and lets the auto-exposure preview a pixel
// without a gpu round trip.
//
// every operator maps unbounded-positive hdr to roughly [0,1]. they are NOT
// gamma-encoded — that happens later in the grade/output stage.
typedef struct {
    int   kind;        // TONEMAP_CURVE_*
    float white;       // white point for the reinhard-extended / filmic ops
    float exposure;    // linear multiplier applied before the curve
} tonemap_curve;
// fill with defaults for a given curve kind. clamps the kind into range.
void  tonemap_curve_init(tonemap_curve *c, int kind);
// set the exposure multiplier (linear, not stops). negative is clamped to 0.
void  tonemap_curve_set_exposure(tonemap_curve *c, float mult);
// apply exposure + the selected operator to a single channel value.
float tonemap_curve_apply1(const tonemap_curve *c, float x);
// apply to an rgb color, channel-wise (the default, preserves hue better than
// luminance-only tonemapping for most game content).
vec3  tonemap_curve_apply(const tonemap_curve *c, vec3 hdr);
// the bare operators, no exposure baked in. exposed so the shader-side
// constants and the selftest can both reach them.
float tonemap_reinhard(float x);
float tonemap_reinhard_ext(float x, float white);
float tonemap_aces(float x);
float tonemap_filmic(float x);
// human-readable name for logging / debug ui.
const char *tonemap_curve_name(int kind);
#endif
