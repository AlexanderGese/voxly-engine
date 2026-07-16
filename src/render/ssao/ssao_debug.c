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
    if (n <= 0) {
        out->min_occl = out->max_occl = out->avg_occl = 0.0f;
        out->dark_frac = 0.0f;
        out->pixels = 0;
        return;
    }

    float mn = 1.0f, mx = 0.0f, sum = 0.0f;
    int dark = 0;
    for (int i = 0; i < n; i++) {
        float v = occl[i];
        if (v < mn) mn = v;
        if (v > mx) mx = v;
        sum += v;
        if (v < 0.5f) dark++;
    }
    out->min_occl  = mn;
    out->max_occl  = mx;
    out->avg_occl  = sum / (float)n;
    out->dark_frac = (float)dark / (float)n;
    out->pixels    = n;
}

int ssaox_debug_stats(const ssaox_pass *p, ssaox_stats *out) {
    if (!p->enabled) return 0;

    int w = p->blur.w, h = p->blur.h;
    glid tex = ssaox_pass_result(p);
    if (!tex || w <= 0 || h <= 0) return 0;

    // readback the R8 occlusion as bytes, then normalize to float.
    unsigned char *bytes = (unsigned char*)malloc((size_t)w * h);
    if (!bytes) return 0;

    // bind via a temporary fbo so we can glReadPixels the texture. cheaper to
    // just attach it and read than to set up glGetTexImage paths everywhere.
    glid fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, tex, 0);
    glReadPixels(0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, bytes);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

    float *occl = (float*)malloc(sizeof(float) * (size_t)w * h);
    if (!occl) { free(bytes); return 0; }
    for (int i = 0; i < w * h; i++) occl[i] = (float)bytes[i] / 255.0f;

    ssaox_debug_stats_cpu(occl, w, h, out);

    free(occl);
    free(bytes);
    return 1;
}

void ssaox_debug_format(const ssaox_stats *st, char *buf, int buflen) {
    snprintf(buf, (size_t)buflen,
             "ssao: avg %.2f min %.2f dark %.0f%% (%d px)",
             st->avg_occl, st->min_occl, st->dark_frac * 100.0f, st->pixels);
}
