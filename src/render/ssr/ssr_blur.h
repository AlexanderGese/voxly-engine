#ifndef RENDER_SSR_BLUR_H
#define RENDER_SSR_BLUR_H

#include "../gl.h"
#include "../../math/vec4.h"

// roughness-aware reflection blur. a single perfect mirror ray is wrong for any
// surface rougher than glass — real glossy reflections spread out with the
// roughness. rather than firing a cone of rays (expensive) we march one sharp
// ray and then blur the half-res reflection buffer by a radius that scales with
// the surface roughness. cheap, and the eye buys it for wet stone / brushed
// metal.
//
// the blur is weighted by each tap's confidence (the reflection alpha) so we
// never bleed a valid reflection out into the empty (alpha 0) regions, which
// would smear haloes off reflection edges.
//
// gpu side owns its own ping output buffer; the cpu side is a plain reference
// over vec4 buffers for the tests.

typedef struct {
    glid fbo;
    glid tex;        // blurred reflection, RGBA16F
    glid prog;       // ssrx_blur.frag, 0 if missing
    int  w, h;
    int  max_radius; // upper bound on the roughness-scaled radius, in texels
} ssrx_blur;

int  ssrx_blur_init(ssrx_blur *b, int w, int h, glid prog, int max_radius);
void ssrx_blur_destroy(ssrx_blur *b);
void ssrx_blur_resize(ssrx_blur *b, int w, int h);

// blur `reflect_tex` into b->tex on the gpu. needs a bound-able fullscreen vao.
// returns b->tex. no-op returning reflect_tex if the shader is missing.
glid ssrx_blur_run(ssrx_blur *b, glid reflect_tex, glid rough_tex, glid fs_vao);

// map a roughness 0..1 to a blur radius in texels, clamped to max_radius. the
// curve is quadratic so smooth surfaces stay crisp and only rough ones spread.
int  ssrx_blur_radius_for(int max_radius, float roughness);

// cpu reference: confidence-weighted box blur over a vec4 reflection buffer.
// `rough` (w*h, may be NULL) drives a per-pixel radius; NULL uses max_radius
// everywhere. in/out must not alias.
void ssrx_blur_cpu(const vec4 *in, const float *rough, vec4 *out,
                   int w, int h, int max_radius);

#endif
