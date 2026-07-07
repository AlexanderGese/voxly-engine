#ifndef RENDER_FXAA_LUMA_H
#define RENDER_FXAA_LUMA_H
#include "fxaa_config.h"
// luma helpers. fxaa is a luma-domain filter: every decision (edge trigger,
// search end, blend direction) is made on a scalar brightness, never on rgb.
// the gpu prepass writes luma into the alpha channel of the source texture so
// the main pass gets it for free; these cpu mirrors of the same math exist for
// the self-test, the cpu reference path and the debug overlay.
// perceptual luma from linear-ish rgb in [0,1]. matches FXAA_LUMA_* weights.
float fxaa_luma_rgb(float r, float g, float b);
// the "fast" luma the inner loop uses. when FXAA_FAST_LUMA_USES_GREEN this is
// just green (fxaa's classic shortcut); otherwise it falls back to the full
// weighted luma. kept separate so prepass/loop weightings can be matched.
float fxaa_luma_fast(float r, float g, float b);
// pack a luma value into the alpha byte of an rgba8 texel the way the prepass
// shader does (round-to-nearest, clamped). returns 0..255.
unsigned char fxaa_luma_pack_alpha(float luma);
// local contrast over the fxaa "plus" neighbourhood: center plus the four
// orthogonal neighbours. returns max-min, which is what the edge trigger
// compares against. nw/ne/sw/se diagonals are not part of the trigger.
float fxaa_luma_contrast(float c, float n, float s, float w, float e);
// blend-direction helper. given the center and its 8 neighbours, estimate the
// local average luma (used by the sub-pixel pass) the same way the shader's
// lumaAverage does: ortho neighbours weighted 2x, diagonals 1x.
float fxaa_luma_average8(float c,
                         float n, float s, float w, float e,
                         float nw, float ne, float sw, float se);
#endif
