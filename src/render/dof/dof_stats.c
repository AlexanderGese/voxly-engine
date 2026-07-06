#include "dof_stats.h"

#include <math.h>
#include <stdio.h>

// bin a signed coc (in texels, normalized to [-1,1] against max) into one of
// DOF_HIST_BINS slots. the centre bin straddles zero (the focal plane).
static int bin_for(float signed_coc, float max_coc) {
    if (max_coc <= 0.0f) return DOF_HIST_BINS / 2;
    float t = signed_coc / max_coc;        // [-1,1]
    if (t < -1.0f) t = -1.0f;
    if (t >  1.0f) t =  1.0f;
    // map [-1,1] -> [0, BINS-1]
    int b = (int)((t * 0.5f + 0.5f) * (DOF_HIST_BINS - 1) + 0.5f);
    if (b < 0) b = 0;
    if (b >= DOF_HIST_BINS) b = DOF_HIST_BINS - 1;
    return b;
}

void dof_stats_sweep(dof_stats *s, const dof_lens *lens,
                     float near, float far, int count, float epsilon) {
    for (int i = 0; i < DOF_HIST_BINS; i++) s->bins[i] = 0;
    s->min_coc = 0.0f;
    s->max_coc = 0.0f;
    s->in_focus = 0;
    s->total = 0;

    if (count < 1) count = 1;
    if (near <= 0.0f) near = 1e-3f;
    if (far <= near)  far = near * 2.0f;

    // log-spaced sweep: depth perception (and coc) is roughly logarithmic, so
    // a linear sweep wastes most samples out in the boring far field.
    float lnear = logf(near);
    float lfar  = logf(far);

    for (int i = 0; i < count; i++) {
        float t = (count > 1) ? (float)i / (float)(count - 1) : 0.0f;
        float dist = expf(lnear + (lfar - lnear) * t);

        float c = dof_coc_signed(lens, dist, lens->max_coc);
        s->bins[bin_for(c, lens->max_coc)]++;

        if (i == 0 || c < s->min_coc) s->min_coc = c;
        if (i == 0 || c > s->max_coc) s->max_coc = c;
        if (fabsf(c) <= epsilon) s->in_focus++;
        s->total++;
    }
}

float dof_stats_hyperfocal(const dof_lens *lens, float coc_limit_texels,
                           float texel_scale) {
    // H ~= f^2 / (N * c) + f, with c the acceptable coc on the sensor. we have
    // the coc limit in texels, so convert back to sensor units first.
    if (coc_limit_texels <= 0.0f || texel_scale <= 0.0f) return INFINITY;

    float c = (coc_limit_texels / texel_scale) * lens->sensor_w;
    float f = lens->focal_len;
    float n = lens->fstop;
    if (n <= 0.0f || c <= 0.0f) return INFINITY;

    return (f * f) / (n * c) + f;
}

float dof_stats_kernel_uniformity(const dof_kernel *k) {
    // drop every tap into a 4x4 grid over the [-1,1] disc bounding box and
    // compare the busiest occupied cell to the quietest occupied cell.
    int grid[4][4] = {{0}};
    for (int i = 0; i < k->count; i++) {
        float x = k->offsets[i].x * 0.5f + 0.5f; // -> [0,1]
        float y = k->offsets[i].y * 0.5f + 0.5f;
        int gx = (int)(x * 4.0f); if (gx > 3) gx = 3; if (gx < 0) gx = 0;
        int gy = (int)(y * 4.0f); if (gy > 3) gy = 3; if (gy < 0) gy = 0;
        grid[gy][gx]++;
    }

    int lo = 0, hi = 0, seen = 0;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int v = grid[y][x];
            if (v == 0) continue; // corner cells fall outside the disc
            if (!seen || v < lo) lo = v;
            if (!seen || v > hi) hi = v;
            seen = 1;
        }
    }
    if (!seen || lo == 0) return 1.0f;
    return (float)hi / (float)lo;
}

int dof_stats_format(const dof_stats *s, char *buf, int cap) {
    if (cap <= 0) return 0;
    int n = snprintf(buf, (size_t)cap,
                     "dof coc[%.2f..%.2f] focus=%d/%d",
                     s->min_coc, s->max_coc, s->in_focus, s->total);
    if (n < 0) { buf[0] = '\0'; return 0; }
    if (n >= cap) n = cap - 1;
    return n;
}
