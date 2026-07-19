#include "ssr_stats.h"

#include <string.h>

void ssrx_stats_reset(ssrx_stats *s) {
    memset(s, 0, sizeof *s);
}

void ssrx_stats_add(ssrx_stats *s, const ssrx_march_result *r, int ray_valid) {
    s->n_rays++;
    if (!ray_valid) return;
    s->n_valid++;
    s->n_steps += r->steps;
    if (r->status == SSRX_MARCH_HIT) s->n_hits++;
    else if (r->status == SSRX_MARCH_SKY) s->n_sky++;
}

float ssrx_stats_hit_ratio(const ssrx_stats *s) {
    if (s->n_valid <= 0) return 0.0f;
    return (float)s->n_hits / (float)s->n_valid;
}

float ssrx_stats_avg_steps(const ssrx_stats *s) {
    if (s->n_rays <= 0) return 0.0f;
    return (float)s->n_steps / (float)s->n_rays;
}

void ssrx_stats_push_timing(ssrx_stats *s, float reflect_ms, float resolve_ms) {
    const float a = 0.1f;
    // first sample seeds the average instead of crawling up from zero.
    if (s->reflect_ms <= 0.0f) s->reflect_ms = reflect_ms;
    else s->reflect_ms = s->reflect_ms * (1.0f - a) + reflect_ms * a;

    if (s->resolve_ms <= 0.0f) s->resolve_ms = resolve_ms;
    else s->resolve_ms = s->resolve_ms * (1.0f - a) + resolve_ms * a;
}
