#ifndef RENDER_TONEMAP_GRADE_H
#define RENDER_TONEMAP_GRADE_H

#include "../../math/vec3.h"
#include "tonemap_config.h"

// color grade, applied to the post-tonemap [0,1] ldr color (before gamma
// encode). this is the cheap creative layer: contrast, saturation, a
// white-balance shove via temp/tint, and the classic lift/gamma/gain knobs.
//
// the cpu version here is the reference. the shader does the same math with
// the same packed coefficients so previews match.

typedef struct {
    float contrast;     // pivot around middle grey. 1 = identity
    float saturation;   // 0 = greyscale, 1 = identity, >1 punchy
    float temperature;  // -1 cool (blue) .. +1 warm (orange)
    float tint;         // -1 green .. +1 magenta
    vec3  lift;         // shadows offset   (added)
    vec3  gamma;        // midtones power    (1/gamma exponent)
    vec3  gain;         // highlights scale  (multiplied)
} tonemap_grade;

// neutral grade: everything identity, lift 0, gamma 1, gain 1.
void tonemap_grade_init(tonemap_grade *g);

// clamp every knob into a sane range so a bad config cant produce nan.
void tonemap_grade_sanitize(tonemap_grade *g);

// the white-balance multiplier the temp/tint knobs resolve to. handy for the
// shader uniform and the selftest. returns an rgb scale centered on white.
vec3 tonemap_grade_white_balance(const tonemap_grade *g);

// apply the full grade to one ldr color. operations compose in the order:
// white-balance -> lift/gamma/gain -> contrast -> saturation. output stays in
// [0,1] (clamped at the end).
vec3 tonemap_grade_apply(const tonemap_grade *g, vec3 c);

// luminance of an ldr color using the rec.709 weights from config.
float tonemap_grade_luma(vec3 c);

#endif
