#include "ssao_kernel.h"

#include <math.h>

// lerp helper. dont want to drag in a whole math util for one line.
static float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

float ssaox_kernel_accel(float t) {
    // square the parameter so we cluster samples near the origin.
    // 0.1 floor keeps a few samples out at the rim, otherwise the ao goes
    // mushy and loses the contact shadow look.
    return lerpf(0.1f, 1.0f, t * t);
}

// one hemisphere sample in tangent space (+z up). returns a point inside the
// unit hemisphere, length scaled by the accel curve and the fractional index.
static vec3 sample_one(rng *r, int i, int count) {
    vec3 s;
    // x,y in [-1,1], z in [0,1] -> points only in the upper hemisphere.
    s.x = rng_frange(r, -1.0f, 1.0f);
    s.y = rng_frange(r, -1.0f, 1.0f);
    s.z = rng_float01(r);

    s = vec3_normalize(s);
    // random radius so samples fill the volume not just the shell
    s = vec3_scale(s, rng_float01(r));

    // bias length toward the center based on how far through the loop we are
    float t = (count > 1) ? (float)i / (float)(count - 1) : 0.0f;
    s = vec3_scale(s, ssaox_kernel_accel(t));
    return s;
}

int ssaox_kernel_build(ssaox_kernel *k, int count, uint64_t seed) {
    if (count < 1) count = 1;
    if (count > SSAOX_KERNEL_MAX) count = SSAOX_KERNEL_MAX;

    rng_init(&k->rnd, seed);
    k->count = count;

    for (int i = 0; i < count; i++) {
        k->samples[i] = sample_one(&k->rnd, i, count);
    }
    return count;
}

void ssaox_kernel_reroll(ssaox_kernel *k, uint64_t seed) {
    int count = k->count;
    ssaox_kernel_build(k, count, seed);
}

int ssaox_kernel_pack(const ssaox_kernel *k, float *out) {
    int n = 0;
    for (int i = 0; i < k->count; i++) {
        out[n++] = k->samples[i].x;
        out[n++] = k->samples[i].y;
        out[n++] = k->samples[i].z;
    }
    return n;
}
