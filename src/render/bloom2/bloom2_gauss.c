#include "bloom2_gauss.h"

#include <math.h>

float bloom2_gauss_eval(float x, float sigma) {
    // unnormalized gaussian. we normalize the whole kernel afterwards so the
    // 1/(sigma*sqrt(2pi)) factor would just cancel — skip it.
    float s2 = sigma * sigma;
    return expf(-(x * x) / (2.0f * s2));
}

void bloom2_gauss_build(bloom2_gauss *g, float sigma) {
    g->taps = BLOOM2_GAUSS_TAPS;

    if (sigma <= 0.0f) {
        // rule of thumb: radius ~= 3 sigma covers >99% of the curve, so go
        // the other way and derive sigma from the radius we have room for.
        sigma = (float)BLOOM2_GAUSS_RADIUS / 3.0f;
        if (sigma < 0.5f) sigma = 0.5f;
    }
    g->sigma = sigma;

    float sum = 0.0f;
    for (int i = 0; i < g->taps; i++) {
        int   off = i - BLOOM2_GAUSS_RADIUS;   // -R .. +R
        float w   = bloom2_gauss_eval((float)off, sigma);
        g->offset[i] = (float)off;
        g->weight[i] = w;
        sum += w;
    }

    // normalize so energy is conserved — bloom that brightens the frame on
    // its own is a bug i chased for an embarrassingly long time.
    if (sum > 0.0f) {
        float inv = 1.0f / sum;
        for (int i = 0; i < g->taps; i++)
            g->weight[i] *= inv;
    }
}

int bloom2_gauss_pack_linear(const bloom2_gauss *g,
                             float *out_weight, float *out_offset) {
    // the center tap stays on its own. pairs to either side get folded into
    // a single linearly-interpolated fetch whose offset is the weighted
    // midpoint of the pair. this halves the bandwidth for the blur.
    int center = BLOOM2_GAUSS_RADIUS;
    int packed = 0;

    // emit center first
    out_weight[packed] = g->weight[center];
    out_offset[packed] = g->offset[center];
    packed++;

    // walk outward in pairs (center+1, center+2), (center+3, center+4), ...
    for (int i = center + 1; i < g->taps; i += 2) {
        float w0 = g->weight[i];
        float w1 = (i + 1 < g->taps) ? g->weight[i + 1] : 0.0f;
        float o0 = g->offset[i];
        float o1 = (i + 1 < g->taps) ? g->offset[i + 1] : o0;

        float wsum = w0 + w1;
        float omid = (wsum > 0.0f) ? (o0 * w0 + o1 * w1) / wsum : o0;

        out_weight[packed] = wsum;
        out_offset[packed] = omid;
        packed++;
    }
    return packed;
}
