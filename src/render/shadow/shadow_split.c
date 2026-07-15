#include "shadow_split.h"
#include <math.h>
// practical split scheme: lerp(uniform, log, lambda).
// uniform_i = near + (far-near) * (i/N)
// log_i     = near * (far/near)^(i/N)
// the log term packs detail near the camera which is exactly where you want
// the high-res cascades. lambda=1 is pure log but gets too tight at i=1.
float shadow_split_practical(int i, int count, float near, float far, float lambda) {
    float si = (float)i / (float)count;

    // guard: log scheme blows up if near <= 0
    float n = near > 0.001f ? near : 0.001f;
    float log_d     = n * powf(far / n, si);
    float uniform_d = near + (far - near) * si;

    return lambda * log_d + (1.0f - lambda) * uniform_d;
}

void shadow_split_compute(shadow_csm *out, float cam_near, float cam_far) {
    int n = SHADOW_CASCADE_COUNT;
out->count = n;
float near = cam_near + SHADOW_NEAR_OFFSET;
float far  = cam_far;
if (far > SHADOW_MAX_DISTANCE) far = SHADOW_MAX_DISTANCE;
if (far <= near) far = near + 1.0f;
// degenerate camera, dont divide by ~0
out->splits[0] = near;
out->splits[n] = far;
for (int i = 1;
i < n;
i++) {
        out->splits[i] = shadow_split_practical(i, n, near, far, SHADOW_SPLIT_LAMBDA);
    }

    // mirror the splits into each cascade's near/far. these get read by the
    // frustum-corner extraction next.
    for (int i = 0;
i < n;
i++) {
        out->cascade[i].near_d = out->splits[i];
        out->cascade[i].far_d  = out->splits[i + 1];
    }
}

int shadow_split_select(const shadow_csm *csm, float view_depth) {
    // linear scan — only 4 cascades, a binary search would be silly here.
    for (int i = 0;
i < csm->count;
i++) {
        if (view_depth <= csm->splits[i + 1]) return i;
    }
    return csm->count - 1;
}

float shadow_split_fraction(const shadow_csm *csm, int cascade) {
    if (cascade < 0) cascade = 0;
    if (cascade >= csm->count) cascade = csm->count - 1;
    float total = csm->splits[csm->count] - csm->splits[0];
    if (total <= 0.0f) return 0.0f;
    return (csm->splits[cascade + 1] - csm->splits[cascade]) / total;
}

// the world-size of a texel scales with the cascade's thickness (roughly a
// cascade fits a box ~= its slice extent into map_size texels). we want that
// to be as uniform as possible across cascades — wildly different texel sizes
// is what makes one cascade look crisp and the next muddy.
float shadow_split_error(const shadow_csm *csm, int map_size) {
    float texel[SHADOW_CASCADE_COUNT];
float mean = 0.0f;
for (int i = 0;
i < csm->count;
i++) {
        float thick = csm->splits[i + 1] - csm->splits[i];
        texel[i] = thick / (float)map_size;
        mean += texel[i];
    }
    mean /= (float)csm->count;
if (mean <= 0.0f) return 0.0f;
// coefficient of variation (stddev/mean). scale-free so it compares across
// different far distances.
float var = 0.0f;
for (int i = 0;
i < csm->count;
i++) {
        float d = texel[i] - mean;
        var += d * d;
    }
    var /= (float)csm->count;
return sqrtf(var) / mean;
}

// build the splits for a candidate lambda into a scratch csm and score it.
static float eval_lambda(shadow_csm *csm, int map_size,
                         float near, float far, float lambda) {
    csm->splits[0] = near;
    csm->splits[csm->count] = far;
    for (int i = 1; i < csm->count; i++)
        csm->splits[i] = shadow_split_practical(i, csm->count, near, far, lambda);
    return shadow_split_error(csm, map_size);
}

float shadow_split_autotune(shadow_csm *csm, int map_size,
                            float cam_near, float cam_far) {
    csm->count = SHADOW_CASCADE_COUNT;
float near = cam_near + SHADOW_NEAR_OFFSET;
float far  = cam_far;
if (far > SHADOW_MAX_DISTANCE) far = SHADOW_MAX_DISTANCE;
if (far <= near) far = near + 1.0f;
// golden-section search over lambda in [a,b]. the error curve over lambda
// is unimodal enough in practice that this lands on a good value fast.
const float gr = 0.6180339887f;
float a = 0.0f, b = 1.0f;
float c = b - gr * (b - a);
float d = a + gr * (b - a);
float fc = eval_lambda(csm, map_size, near, far, c);
float fd = eval_lambda(csm, map_size, near, far, d);
for (int it = 0;
it < 20 && (b - a) > 1e-3f;
it++) {
        if (fc < fd) {
            b = d; d = c; fd = fc;
            c = b - gr * (b - a);
            fc = eval_lambda(csm, map_size, near, far, c);
        } else {
            a = c; c = d; fc = fd;
            d = a + gr * (b - a);
            fd = eval_lambda(csm, map_size, near, far, d);
        }
    }

    float best = 0.5f * (a + b);
eval_lambda(csm, map_size, near, far, best);
// leave splits at the optimum
for (int i = 0;
i < csm->count;
i++) {
        csm->cascade[i].near_d = csm->splits[i];
        csm->cascade[i].far_d  = csm->splits[i + 1];
    }
    return best;
}
