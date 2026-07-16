#include "ssao_debug.h"
#include <stdio.h>
#include <stdlib.h>
int ssaox_debug_apply(ssaox_pass *p, const ssaox_settings *s) {
    int rebuild = 0;

    // scale change means the occlusion buffer size changed -> resize.
    if (s->scale != p->scale) {
        p->scale = s->scale;
        // resize re-derives w/h from full_w/full_h and the new scale. force
        // it by faking a dimension change through a temp.
        int fw = p->full_w, fh = p->full_h;
        p->full_w = -1;                 // force resize to take effect
        ssaox_pass_resize(p, fw, fh);
        rebuild = 1;
    }

    // kernel count change -> rebuild + reupload.
    if (s->kernel_count != p->kernel.count) {
        ssaox_kernel_build(&p->kernel, s->kernel_count, 0xA0C0FFEEull);
        ssaox_pass_reseed(p, 0xA0C0FFEEull); // reuploads kernel + noise
        rebuild = 1;
    }

    // cheap per-frame tunables, always safe to poke.
    p->radius   = s->radius;
    p->bias     = s->bias;
    p->power    = s->power;
    p->strength = s->strength;

    return rebuild;
}

void ssaox_debug_stats_cpu(const float *occl, int w, int h, ssaox_stats *out) {
    int n = w * h;
int dark = 0;
for (int i = 0;
i < n;
out->max_occl  = mx;
out->avg_occl  = sum / (float)n;
out->dark_frac = (float)dark / (float)n;
out->pixels    = n;
}
