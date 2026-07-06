#include "dof_stats.h"
#include <math.h>
#include <stdio.h>
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
    for (int i = 0;
i < DOF_HIST_BINS;
i++) s->bins[i] = 0;
s->min_coc = 0.0f;
s->max_coc = 0.0f;
s->in_focus = 0;
s->total = 0;
if (count < 1) count = 1;
if (near <= 0.0f) near = 1e-3f;
if (far <= near)  far = near * 2.0f;
float lnear = logf(near);
float lfar  = logf(far);
for (int i = 0;
i < count;
float c = (coc_limit_texels / texel_scale) * lens->sensor_w;
float f = lens->focal_len;
float n = lens->fstop;
if (n <= 0.0f || c <= 0.0f) return INFINITY;
return (f * f) / (n * c) + f;
int n = snprintf(buf, (size_t)cap,
                     "dof coc[%.2f..%.2f] focus=%d/%d",
                     s->min_coc, s->max_coc, s->in_focus, s->total);
return n;
}
