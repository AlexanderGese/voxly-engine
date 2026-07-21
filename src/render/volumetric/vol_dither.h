#ifndef RENDER_VOLUMETRIC_DITHER_H
#define RENDER_VOLUMETRIC_DITHER_H
#include "vol_config.h"
#include "../gl.h"
// the per-pixel ray-start jitter. a bayer ordered-dither matrix, normalized to
// [0,1), tiled across the screen. each ray nudges its first sample by
// dither(pixel) * step_len so the discrete steps decorrelate between
// neighbours and the banding becomes noise the blur can swallow.
//
// we keep the matrix on the cpu (the raymarch reference reads it directly) and
// also upload it as an R8/R16F texture for the gpu march.
typedef struct {
    int   dim;                       // VOL_DITHER_DIM
    float texels[VOL_DITHER_TEXELS]; // row-major, values in [0,1)
    glid  tex;                       // uploaded R16F texture, 0 if not uploaded
} volumetric_dither;
// build the bayer matrix for the configured dimension. only powers of two are
// meaningful for bayer; non-pow2 dims fall back to a hashed pattern.
void  volumetric_dither_build(volumetric_dither *d);
// upload (or re-upload) as a single-channel float texture, NEAREST + REPEAT.
glid  volumetric_dither_upload(volumetric_dither *d);
void  volumetric_dither_free(volumetric_dither *d);
// sample the matrix at integer pixel coords (wraps). matches the glsl texelFetch
// with a modulo, so cpu and gpu marches start their rays at the same offset.
float volumetric_dither_at(const volumetric_dither *d, int px, int py);
// how many tiles the matrix spans across a w x h target — handed to the shader
// as a uv multiplier just like the ssao noise scale.
void  volumetric_dither_scale(const volumetric_dither *d, int w, int h,
                              float out[2]);
#endif
