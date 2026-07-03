#ifndef RENDER_BLOOM2_BLUR_H
#define RENDER_BLOOM2_BLUR_H

#include "../gl.h"
#include "bloom2_config.h"
#include "bloom2_gauss.h"
#include "bloom2_target.h"
#include "bloom2_quad.h"

// separable gaussian blur fallback. the mip-chain in bloom2_pass is the main
// path, but on some intel drivers the linear-sampling downsample produced
// visible banding, so we keep a plain two-pass separable blur around as a
// "safe mode". it ping-pongs between two equal-sized targets.
//
// this path uploads the cpu-precomputed (linear-packed) gaussian taps as a
// uniform array, so it actually exercises bloom2_gauss.

typedef struct {
    glid prog_h;          // horizontal pass program
    glid prog_v;          // vertical pass program
    bloom2_target scratch; // ping target, same size as the input mip
    bloom2_gauss  gauss;
    float packed_weight[BLOOM2_GAUSS_RADIUS + 1];
    float packed_offset[BLOOM2_GAUSS_RADIUS + 1];
    int   packed_taps;
    int   ready;
} bloom2_blur;

// build the blur for targets of size w x h. sigma <= 0 -> default.
int  bloom2_blur_init(bloom2_blur *bl, int w, int h, float sigma);
void bloom2_blur_destroy(bloom2_blur *bl);

// rebuild the gaussian for a new sigma without touching the gl objects.
void bloom2_blur_set_sigma(bloom2_blur *bl, float sigma);

// blur `io` in place: horizontal into scratch, vertical back into io. `io`
// must be the same size the blur was created for.
void bloom2_blur_apply(bloom2_blur *bl, bloom2_target *io, const bloom2_quad *q);

#endif
