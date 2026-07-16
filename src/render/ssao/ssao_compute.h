#ifndef RENDER_SSAO_COMPUTE_H
#define RENDER_SSAO_COMPUTE_H

#include "ssao_kernel.h"
#include "ssao_noise.h"
#include "../../math/mat4.h"
#include "../../math/vec3.h"

// cpu reference implementation of the occlusion pass. operates on plain
// view-space position + normal buffers instead of gl textures. this is the
// ground truth the glsl is supposed to match, and it's what the tests poke.
//
// it's not fast and not meant to be — it's a few hundred pixels in a unit
// test, not a frame. if you ever run it on a real framebuffer, go make a
// coffee.

typedef struct {
    int   w, h;          // buffer dimensions
    mat4  proj;          // projection (view -> clip)
    float radius;
    float bias;
    float power;
} ssaox_compute_params;

// compute one pixel's occlusion factor (1 = lit, 0 = fully occluded).
// pos    : per-pixel view-space positions, w*h vec3
// nrm    : per-pixel view-space normals,   w*h vec3
// px,py  : the pixel to evaluate
// kernel + noise orient and jitter the samples. result already has the power
// curve applied.
float ssaox_compute_pixel(const ssaox_compute_params *prm,
                          const ssaox_kernel *k, const ssaox_noise *noise,
                          const vec3 *pos, const vec3 *nrm,
                          int px, int py);

// fill an entire w*h occlusion buffer (single channel float). does the raw
// occlusion only — call ssaox_blur_cpu afterwards to match the gpu path.
void ssaox_compute_buffer(const ssaox_compute_params *prm,
                          const ssaox_kernel *k, const ssaox_noise *noise,
                          const vec3 *pos, const vec3 *nrm,
                          float *out_occl);

#endif
