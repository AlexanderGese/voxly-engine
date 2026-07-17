#ifndef RENDER_SSAO_NOISE_H
#define RENDER_SSAO_NOISE_H

#include "ssao_config.h"
#include "../../math/vec3.h"
#include "../../math/rng.h"

// the rotation noise tile. each texel is a unit vector in the tangent plane
// (z == 0) used to randomly rotate the sample kernel per fragment. tiling a
// tiny texture across the screen + blurring afterwards hides the repetition.
//
// we keep a cpu copy of the texels so tests dont need a gl context, and so
// we can rebuild the gl texture after a context loss.

typedef struct {
    vec3 texels[SSAOX_NOISE_TEXELS];   // z always 0, x/y in [-1,1]
    int  dim;                          // == SSAOX_NOISE_DIM
    glid tex;                          // gl handle, 0 if not uploaded
} ssaox_noise;

// fill the cpu texel array from a seed. does not touch gl.
void ssaox_noise_build(ssaox_noise *n, uint64_t seed);

// (re)upload the cpu texels into a GL_RG16F (well, RGB) texture, NEAREST +
// REPEAT so it tiles. needs a live gl context. returns the texture handle.
glid ssaox_noise_upload(ssaox_noise *n);

// free the gl texture (cpu copy stays).
void ssaox_noise_free(ssaox_noise *n);

// how many times the tile repeats across a (w,h) target. handy for setting
// the u_noise_scale uniform in the shader. writes 2 floats.
void ssaox_noise_scale(const ssaox_noise *n, int w, int h, float out[2]);

#endif
