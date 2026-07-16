#ifndef RENDER_SSAO_CONFIG_H
#define RENDER_SSAO_CONFIG_H

// tunables for the screen-space ambient occlusion subsystem.
// this is a *separate* module from render/post/ssao.c (which is the old stub).
// prefix is ssaox_ so the two dont fight over symbols while i migrate.
//
// nothing here is sacred — radius/bias get hand-tuned per scene anyway.

#include "../gl.h"

// kernel: hemisphere sample points in tangent space. 16 is the sweet spot
// for our potato target. 32 looks marginally better and costs ~2x. 64 is
// vanity. keep it a power of two so the shader loop unrolls clean.
#define SSAOX_KERNEL_MAX        64
#define SSAOX_KERNEL_DEFAULT    16

// rotation noise tile. small repeating texture we tile across the screen to
// randomly rotate the kernel per-fragment. 4x4 is plenty, the blur smears
// the repeating pattern out anyway.
#define SSAOX_NOISE_DIM         4
#define SSAOX_NOISE_TEXELS      (SSAOX_NOISE_DIM * SSAOX_NOISE_DIM)

// the occlusion buffer is rendered at half res by default. ao is low
// frequency, nobody notices, and it halves the fill cost. set scale to 1
// if you have gpu to burn.
#define SSAOX_DEFAULT_SCALE     2

// separable blur kernel radius (in texels, each side). 2 -> 5x5 box.
#define SSAOX_BLUR_RADIUS       2

// sane defaults for the sampling math
#define SSAOX_DEFAULT_RADIUS    0.5f
#define SSAOX_DEFAULT_BIAS      0.025f
#define SSAOX_DEFAULT_POWER     1.0f    // contrast curve, occ = pow(occ, power)
#define SSAOX_DEFAULT_STRENGTH  1.0f    // final blend amount 0..1

// range check: when the depth difference between the fragment and the sample
// is bigger than the radius we fade the sample's contribution so distant
// geometry behind a near surface doesnt darken it. this is the smoothstep
// width as a fraction of radius.
#define SSAOX_RANGE_FALLOFF     1.0f

// how many texture units we expect to bind. depth, normal, noise.
#define SSAOX_TEX_UNIT_DEPTH    0
#define SSAOX_TEX_UNIT_NORMAL   1
#define SSAOX_TEX_UNIT_NOISE    2
#define SSAOX_TEX_UNIT_OCCL     0   // for the blur pass

// shader paths. these live next to the rest under shaders/. the .frag's are
// optional at runtime — if theyre missing we just disable the gpu path and
// the cpu reference still works for tests.
#define SSAOX_VERT_PATH         "shaders/post_passthrough.vert"
#define SSAOX_FRAG_OCCL_PATH    "shaders/ssaox_occlude.frag"
#define SSAOX_FRAG_BLUR_PATH    "shaders/ssaox_blur.frag"

#endif
