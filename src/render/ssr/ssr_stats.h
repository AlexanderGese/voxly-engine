#ifndef RENDER_SSR_STATS_H
#define RENDER_SSR_STATS_H

#include "ssr_march.h"

// lightweight telemetry the cpu march/resolve path can accumulate, and the gpu
// pass can stash its per-frame timings into. handy for the debug overlay and
// for catching "why is ssr suddenly hammering the gpu" regressions.
//
// the gpu pass cant cheaply read back hit ratios (no readback per frame, thanks)
// so its counters come from the last cpu validation run; the timings are real.

typedef struct {
    // counts (cpu path fills these; gpu leaves them zero unless validated)
    long n_rays;          // rays attempted
    long n_valid;         // rays that werent culled at build time
    long n_hits;          // rays that found a refined crossing
    long n_sky;           // crossings that landed on the far plane
    long n_steps;         // total linear steps walked (perf proxy)

    // gpu timings (ms), exponential moving average
    float reflect_ms;
    float resolve_ms;
} ssrx_stats;

// zero everything.
void ssrx_stats_reset(ssrx_stats *s);

// fold one march outcome into the counters.
void ssrx_stats_add(ssrx_stats *s, const ssrx_march_result *r, int ray_valid);

// hit ratio over valid rays, 0 if none. the headline number.
float ssrx_stats_hit_ratio(const ssrx_stats *s);

// mean linear steps per attempted ray. tells you if the march is thrashing.
float ssrx_stats_avg_steps(const ssrx_stats *s);

// blend a fresh gpu timing into the moving average (alpha ~0.1 for stability).
void ssrx_stats_push_timing(ssrx_stats *s, float reflect_ms, float resolve_ms);

#endif
