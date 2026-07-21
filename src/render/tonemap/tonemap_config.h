#ifndef RENDER_TONEMAP_CONFIG_H
#define RENDER_TONEMAP_CONFIG_H

// tunables for the hdr tonemap + color grade pass. this runs right at the
// end of the frame: take the rgba16f scene, expose it, push it through a
// tonemap curve, grade it, slap a lut on top, write ldr to the backbuffer.
//
// the old code just did pow(c, 1/2.2) and called it a day. this is the
// grown-up version. magic numbers below are mostly from the aces fit and
// from staring at the histogram until it looked right.

// which tonemap curve presets we know how to build. these map to a knob in
// tonemap_params; the shader gets a flattened curve so it never branches.
#define TONEMAP_CURVE_LINEAR     0   // clamp only, for debugging
#define TONEMAP_CURVE_REINHARD   1   // x/(1+x), cheap, washes out highlights
#define TONEMAP_CURVE_REINHARD_X 2   // extended reinhard with a white point
#define TONEMAP_CURVE_ACES       3   // narkowicz aces fit, the default
#define TONEMAP_CURVE_FILMIC     4   // hable/uncharted2 filmic
#define TONEMAP_CURVE_COUNT      5

#define TONEMAP_DEFAULT_CURVE    TONEMAP_CURVE_ACES

// exposure. we expose in stops (ev) and convert to a linear multiplier with
// 2^ev. middle grey is the anchor the auto-exposure tries to land on.
#define TONEMAP_DEFAULT_EV         0.0f
#define TONEMAP_MIN_EV            -8.0f
#define TONEMAP_MAX_EV             8.0f
#define TONEMAP_MIDDLE_GREY        0.18f

// auto-exposure adapts toward the scene's average luma. these bound how dark
// or bright a scene we'll let the eye adapt to, and how fast it moves. speed
// is in ev/second, and we use a different rate for brightening vs darkening
// because real eyes adapt to dark slower than to light.
#define TONEMAP_AE_MIN_LUMA        0.002f
#define TONEMAP_AE_MAX_LUMA        4.0f
#define TONEMAP_AE_SPEED_UP        2.5f   // adapting to a brighter scene
#define TONEMAP_AE_SPEED_DOWN      1.0f   // adapting to a darker scene

// extended-reinhard white point: the luminance that maps to pure white.
#define TONEMAP_DEFAULT_WHITE      4.0f

// output gamma. we encode to roughly srgb. 2.2 is the cheap approximation;
// if you want the real piecewise srgb curve flip the define in the shader.
#define TONEMAP_DEFAULT_GAMMA      2.2f

// color grade defaults. lift/gamma/gain are the classic three-way grade.
#define TONEMAP_DEFAULT_CONTRAST   1.0f
#define TONEMAP_DEFAULT_SATURATION 1.0f
#define TONEMAP_DEFAULT_TEMP       0.0f   // -1 cool .. +1 warm
#define TONEMAP_DEFAULT_TINT       0.0f   // -1 green .. +1 magenta

// 3d lut. cube luts are typically 16, 32 or 64 on a side. we cap the edge so
// a malformed file cant ask us to allocate a gigabyte.
#define TONEMAP_LUT_MAX_DIM        64
#define TONEMAP_LUT_DEFAULT_DIM    33   // resolve/cube default

// rec.709 luma weights. used all over for luminance and saturation.
#define TONEMAP_LUMA_R             0.2126f
#define TONEMAP_LUMA_G             0.7152f
#define TONEMAP_LUMA_B             0.0722f

#endif
