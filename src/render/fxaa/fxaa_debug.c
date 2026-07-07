#include "fxaa_debug.h"
#include "fxaa_luma.h"
#include "fxaa_quality.h"

#include <stdio.h>
#include <stdlib.h>

void fxaa_debug_stats_cpu(const float *luma, int w, int h,
                          float edge_threshold, fxaa_stats *out) {
    out->pixels = 0;
    out->avg_luma = out->min_luma = out->max_luma = 0.0f;
    out->edge_frac = 0.0f;
    if (!luma || w <= 0 || h <= 0) return;

    float mn = 1.0f, mx = 0.0f, sum = 0.0f;
    int edges = 0, counted = 0;

    // skip the 1-texel border so the plus-neighbourhood fetch stays in bounds.
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            float c = luma[y * w + x];
            float n = luma[(y - 1) * w + x];
            float s = luma[(y + 1) * w + x];
            float wl = luma[y * w + (x - 1)];
            float e = luma[y * w + (x + 1)];

            if (c < mn) mn = c;
            if (c > mx) mx = c;
            sum += c;

            float contrast = fxaa_luma_contrast(c, n, s, wl, e);
            float hi = c;
            if (n > hi) hi = n;
            if (s > hi) hi = s;
            if (wl > hi) hi = wl;
            if (e > hi) hi = e;
            if (contrast >= edge_threshold * hi) edges++;
            counted++;
        }
    }

    if (counted <= 0) return;
    out->pixels   = counted;
    out->avg_luma = sum / (float)counted;
    out->min_luma = mn;
    out->max_luma = mx;
    out->edge_frac = (float)edges / (float)counted;
}

int fxaa_debug_stats(const fxaa *f, fxaa_stats *out) {
    if (!fxaa_is_active(f) || !f->target.tex) return 0;
    int w = f->target.w, h = f->target.h;
    if (w <= 0 || h <= 0) return 0;

    // readback rgba8; we only care about alpha (the packed luma).
    unsigned char *px = (unsigned char*)malloc((size_t)w * h * 4);
    if (!px) return 0;

    glid fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, f->target.tex, 0);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, px);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

    float *luma = (float*)malloc(sizeof(float) * (size_t)w * h);
    if (!luma) { free(px); return 0; }
    for (int i = 0; i < w * h; i++)
        luma[i] = (float)px[i * 4 + 3] / 255.0f;

    fxaa_debug_stats_cpu(luma, w, h, f->params.edge_threshold, out);

    free(luma);
    free(px);
    return 1;
}

void fxaa_debug_format(const fxaa *f, const fxaa_stats *st,
                       char *buf, int buflen) {
    snprintf(buf, (size_t)buflen,
             "fxaa[%s]: edges %.1f%% luma avg %.2f (%d px)",
             fxaa_quality_name(f->params.quality),
             st->edge_frac * 100.0f, st->avg_luma, st->pixels);
}
