#include "dof_kernel.h"

#include <math.h>

// vogel spiral: the i-th of n points sits at radius sqrt(i/n) and angle
// i*golden_angle. the sqrt keeps the areal density uniform (otherwise the
// centre crowds), the golden angle keeps successive points maximally apart.
// this is the same trick sunflowers use to pack seeds, hence the name.

// no M_PI under -std=c11, and the rest of the engine just inlines the literal,
// so we do the same.
#define DOF_PI  3.14159265358979f

float dof_kernel_golden_angle(void) {
    // pi * (3 - sqrt(5)). ~2.39996 rad.
    return DOF_PI * (3.0f - sqrtf(5.0f));
}

int dof_kernel_build(dof_kernel *k, int count) {
    if (count < 1) count = 1;
    if (count > DOFX_KERNEL_MAX) count = DOFX_KERNEL_MAX;
    k->count = count;

    float ga = dof_kernel_golden_angle();

    for (int i = 0; i < count; i++) {
        // +0.5 bias so the very first tap isnt dead-centre (a centre tap is
        // just the source pixel, wasted in a gather).
        float fi = ((float)i + 0.5f) / (float)count;
        float r  = sqrtf(fi);
        float th = (float)i * ga;

        k->offsets[i] = vec2_new(r * cosf(th), r * sinf(th));

        // bokeh has a bright rim (the "donut" from a real lens with a central
        // obstruction), so weight the outer taps a touch heavier. linear ramp
        // from 0.6 at centre to 1.0 at the rim reads close enough.
        k->weight[i] = 0.6f + 0.4f * r;
    }
    return count;
}

float dof_kernel_weight_sum(const dof_kernel *k) {
    float s = 0.0f;
    for (int i = 0; i < k->count; i++) s += k->weight[i];
    return s;
}

int dof_kernel_pack(const dof_kernel *k, float *out) {
    int n = 0;
    for (int i = 0; i < k->count; i++) {
        out[n++] = k->offsets[i].x;
        out[n++] = k->offsets[i].y;
    }
    return n;
}

float dof_kernel_max_radius(const dof_kernel *k) {
    float m = 0.0f;
    for (int i = 0; i < k->count; i++) {
        float r = vec2_length(k->offsets[i]);
        if (r > m) m = r;
    }
    return m;
}
