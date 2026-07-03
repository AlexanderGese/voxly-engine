#ifndef RENDER_BLOOM2_GAUSS_H
#define RENDER_BLOOM2_GAUSS_H

#include "bloom2_config.h"

// precomputed separable gaussian. this is the fallback blur path used when
// the gpu mip chain isnt available (some drivers choke on the linear
// downsample trick). we still compute it on the cpu and upload as uniforms.

typedef struct {
    int   taps;                       // odd, == BLOOM2_GAUSS_TAPS
    float weight[BLOOM2_GAUSS_TAPS];  // normalized, sums to 1
    float offset[BLOOM2_GAUSS_TAPS];  // texel offsets, center is 0
    float sigma;
} bloom2_gauss;

// build a normalized gaussian with the given sigma. sigma <= 0 picks a sane
// default derived from the radius.
void bloom2_gauss_build(bloom2_gauss *g, float sigma);

// collapse adjacent taps using linear-sampling so an N-tap kernel only needs
// ceil(N/2) texture fetches. writes packed weight/offset pairs into out_*
// and returns the packed tap count. classic rastergrid optimization.
int  bloom2_gauss_pack_linear(const bloom2_gauss *g,
                              float *out_weight, float *out_offset);

// evaluate the continuous gaussian at x for a given sigma. exposed for tests
// and for whoever wants to sanity-check the discrete weights.
float bloom2_gauss_eval(float x, float sigma);

#endif
