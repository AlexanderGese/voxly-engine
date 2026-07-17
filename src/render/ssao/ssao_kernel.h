#ifndef RENDER_SSAO_KERNEL_H
#define RENDER_SSAO_KERNEL_H

#include "ssao_config.h"
#include "../../math/vec3.h"
#include "../../math/rng.h"

// the sample kernel: a set of points inside the unit hemisphere oriented
// around +z (tangent space). the shader transforms each by the per-fragment
// TBN and adds it to the view-space position to probe for occluders.
//
// two things make this not-garbage:
// - points are biased toward the origin (more samples close = sharper ao)
// - the noise texture randomly rotates the whole set per fragment

typedef struct {
    vec3 samples[SSAOX_KERNEL_MAX];
    int  count;
    rng  rnd;        // kept around so re-rolls are reproducible
} ssaox_kernel;

// build `count` hemisphere samples from `seed`. count is clamped to
// SSAOX_KERNEL_MAX. returns the count actually used.
int  ssaox_kernel_build(ssaox_kernel *k, int count, uint64_t seed);

// re-scatter the existing kernel with a fresh seed, keeping count.
void ssaox_kernel_reroll(ssaox_kernel *k, uint64_t seed);

// flatten samples into a tightly packed float array (count*3 floats) for
// glUniform3fv. `out` must hold at least count*3 floats. returns floats written.
int  ssaox_kernel_pack(const ssaox_kernel *k, float *out);

// the acceleration curve used to bias samples toward the origin. exposed so
// tests can pin it. lerp(0.1, 1.0, t*t).
float ssaox_kernel_accel(float t);

#endif
