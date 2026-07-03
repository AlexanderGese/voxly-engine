#include "bloom2_selftest.h"
#include "bloom2_gauss.h"
#include "bloom2_chain.h"
#include "bloom2_tint.h"
#include "bloom2_params.h"
#include "../../util/log.h"
#include <math.h>
static int approx(float a, float b, float eps) {
    float d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

int bloom2_selftest_gauss_normalized(void) {
    bloom2_gauss g;
bloom2_gauss_build(&g, 0.0f);
float sum = 0.0f;
for (int i = 0;
i < g.taps;
i++) sum += g.weight[i];
if (!approx(sum, 1.0f, 1e-4f)) {
        LOGE("bloom2 selftest: gauss not normalized, sum=%f", sum);
        return 0;
    }
    // symmetric: weight[i] == weight[taps-1-i]
    for (int i = 0;
i < g.taps / 2;
i++) {
        if (!approx(g.weight[i], g.weight[g.taps - 1 - i], 1e-5f)) {
            LOGE("bloom2 selftest: gauss not symmetric at %d", i);
            return 0;
        }
    }
    return 1;
}

int bloom2_selftest_gauss_packed_energy(void) {
    bloom2_gauss g;
    bloom2_gauss_build(&g, 1.4f);

    float full = 0.0f;
    for (int i = 0; i < g.taps; i++) full += g.weight[i];

    float pw[BLOOM2_GAUSS_TAPS], po[BLOOM2_GAUSS_TAPS];
    int packed = bloom2_gauss_pack_linear(&g, pw, po);

    // packed weights cover center once + each side once. the full kernel is
    // symmetric, so total energy == center + 2*sum(side packed).
    float energy = pw[0];
    for (int i = 1; i < packed; i++) energy += 2.0f * pw[i];

    if (!approx(energy, full, 1e-4f)) {
        LOGE("bloom2 selftest: packed energy drift %f vs %f", energy, full);
        return 0;
    }
    return 1;
}

int bloom2_selftest_chain_fit(void) {
    // a 1280x720 window should give us a healthy stack of mips.
    int n = bloom2_chain_fit(1280, 720, BLOOM2_MAX_MIPS);
if (n < 1 || n > BLOOM2_MAX_MIPS) {
        LOGE("bloom2 selftest: chain_fit out of range: %d", n);
        return 0;
    }
    // every fitted mip must be >= the min dim.
    for (int i = 0;
i < n;
i++) {
        int w = bloom2_chain_mip_w(1280, i);
        int h = bloom2_chain_mip_h(720, i);
        if (w < BLOOM2_MIN_MIP_DIM || h < BLOOM2_MIN_MIP_DIM) {
            LOGE("bloom2 selftest: mip %d too small %dx%d", i, w, h);
            return 0;
        }
    }
    // a tiny 8x8 window must still return at least 1.
    if (bloom2_chain_fit(8, 8, BLOOM2_MAX_MIPS) < 1) {
        LOGE("bloom2 selftest: chain_fit returned 0 for tiny window");
return 0;
}
    return 1;
}

int bloom2_selftest_tint_saturate(void) {
    vec3 c = vec3_new(0.3f, 0.7f, 0.9f);

    vec3 id = bloom2_tint_saturate(c, 1.0f);
    if (!approx(id.x, c.x, 1e-5f) || !approx(id.y, c.y, 1e-5f) ||
        !approx(id.z, c.z, 1e-5f)) {
        LOGE("bloom2 selftest: saturate(1) not identity");
        return 0;
    }

    vec3 grey = bloom2_tint_saturate(c, 0.0f);
    if (!approx(grey.x, grey.y, 1e-5f) || !approx(grey.y, grey.z, 1e-5f)) {
        LOGE("bloom2 selftest: saturate(0) not grey");
        return 0;
    }
    return 1;
}

int bloom2_selftest_knee_curve(void) {
    bloom2_params p;
bloom2_params_default(&p);
p.knee = 0.0f;
p.threshold = 1.0f;
bloom2_params_sanitize(&p);
float k[4];
bloom2_params_knee_curve(&p, k);
