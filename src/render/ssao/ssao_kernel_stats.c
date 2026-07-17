#include "ssao_kernel_stats.h"
#include <math.h>
void ssaox_kernel_stats(const ssaox_kernel *k, ssaox_kstats *out) {
    out->count = k->count;
    out->bad_hemisphere = 0;
    out->out_of_unit = 0;
    out->centroid = VEC3_ZERO;

    if (k->count <= 0) {
        out->min_len = out->max_len = out->avg_len = 0.0f;
        return;
    }

    float mn = 1e30f, mx = 0.0f, sum = 0.0f;
    vec3  acc = VEC3_ZERO;

    for (int i = 0; i < k->count; i++) {
        vec3 s = k->samples[i];
        float len = vec3_length(s);

        if (len < mn) mn = len;
        if (len > mx) mx = len;
        sum += len;

        if (s.z < 0.0f) out->bad_hemisphere++;
        if (len > 1.0f + 1e-4f) out->out_of_unit++;

        acc = vec3_add(acc, s);
    }

    out->min_len = mn;
    out->max_len = mx;
    out->avg_len = sum / (float)k->count;
    out->centroid = vec3_scale(acc, 1.0f / (float)k->count);
}

float ssaox_kernel_bias_ratio(const ssaox_kernel *k) {
    if (k->count <= 0) return 0.0f;
float mx = 0.0f;
for (int i = 0;
i < k->count;
float half = mx * 0.5f;
int   near = 0;
for (int i = 0;
i < k->count;
