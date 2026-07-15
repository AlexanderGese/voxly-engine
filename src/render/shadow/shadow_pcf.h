#ifndef RENDER_SHADOW_PCF_H
#define RENDER_SHADOW_PCF_H

#include "shadow_types.h"
#include "../gl.h"

// pcf (percentage closer filtering) support. most of the actual filtering
// happens in the fragment shader, but the cpu side owns the sample offsets,
// the per-cascade bias, and pushing all of it into uniforms.

// one filter tap: offset in texels + its weight. we precompute a gaussian-ish
// kernel once instead of branching in the shader.
typedef struct {
    float dx, dy;
    float weight;
} shadow_pcf_tap;

typedef struct {
    shadow_pcf_tap taps[(2 * SHADOW_PCF_RADIUS + 1) * (2 * SHADOW_PCF_RADIUS + 1)];
    int   count;
    float inv_size;     // 1/map_size, the texel step in uv
} shadow_pcf_kernel;

// build the kernel for a given map size. gaussian weights, normalized to 1.
void  shadow_pcf_build(shadow_pcf_kernel *k, int map_size);

// alternative kernel: a poisson-ish disk instead of a regular grid. the
// irregular spacing turns the leftover banding into noise, which the eye
// forgives more readily than stripes. same tap count as the grid build.
void  shadow_pcf_build_poisson(shadow_pcf_kernel *k, int map_size);

// rotate a kernel's taps by `angle` radians around the origin. callers use a
// per-cascade or per-frame angle so the noise pattern doesnt sit still.
void  shadow_pcf_rotate(shadow_pcf_kernel *k, float angle);

// estimate the world-space penumbra radius this kernel produces in a cascade.
// purely informational — handy when balancing radius vs cascade size.
float shadow_pcf_penumbra(const shadow_pcf_kernel *k, const shadow_csm *csm,
                          int cascade);

// slope-scaled depth bias for a cascade given the surface normal vs light.
// grazing angles need more bias or they shimmer with acne.
float shadow_pcf_bias(const shadow_csm *csm, int cascade, vec3 normal);

// push the kernel + cascade matrices + splits into the lighting shader.
void  shadow_pcf_upload(glid prog, const shadow_pcf_kernel *k,
                        const shadow_csm *csm);

#endif
