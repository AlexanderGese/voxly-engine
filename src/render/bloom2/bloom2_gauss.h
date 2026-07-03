#ifndef RENDER_BLOOM2_GAUSS_H
#define RENDER_BLOOM2_GAUSS_H
#include "bloom2_config.h"
// precomputed separable gaussian. this is the fallback blur path used when
typedef struct {
    int   taps;                       // odd, == BLOOM2_GAUSS_TAPS
    float weight[BLOOM2_GAUSS_TAPS];  // normalized, sums to 1
    float offset[BLOOM2_GAUSS_TAPS];  // texel offsets, center is 0
    float sigma;
} bloom2_gauss;
void bloom2_gauss_build(bloom2_gauss *g, float sigma);
int  bloom2_gauss_pack_linear(const bloom2_gauss *g,
                              float *out_weight, float *out_offset);
float bloom2_gauss_eval(float x, float sigma);
#endif
