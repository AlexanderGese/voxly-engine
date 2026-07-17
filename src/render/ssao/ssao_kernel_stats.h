#ifndef RENDER_SSAO_KERNEL_STATS_H
#define RENDER_SSAO_KERNEL_STATS_H

#include "ssao_kernel.h"

// introspection over a built kernel. mostly used to assert in tests that the
// scatter is sane: all samples in the +z hemisphere, lengths inside the unit
// sphere, biased toward the origin, not all clumped on one side. caught a
// fencepost in the accel curve once with this, kept it around.

typedef struct {
    float min_len;        // shortest sample (should be > 0)
    float max_len;        // longest (should be <= 1)
    float avg_len;        // mean length — expect < 0.6 due to the accel bias
    vec3  centroid;       // mean position; x,y near 0, z positive
    int   bad_hemisphere; // count of samples with z < 0 (should be 0)
    int   out_of_unit;    // count with length > 1 + eps (should be 0)
    int   count;
} ssaox_kstats;

// compute the stats over the whole kernel.
void ssaox_kernel_stats(const ssaox_kernel *k, ssaox_kstats *out);

// quick pass/fail: hemisphere clean, lengths bounded, centroid leaning +z.
// returns 1 if the kernel looks healthy.
int  ssaox_kernel_ok(const ssaox_kernel *k);

// how front-loaded the lengths are: fraction of samples with length < half
// the max length. higher = more clustered near the origin (what we want).
float ssaox_kernel_bias_ratio(const ssaox_kernel *k);

#endif
