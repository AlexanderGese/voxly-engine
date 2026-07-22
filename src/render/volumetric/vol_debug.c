#include "vol_debug.h"
#include "vol_medium.h"

#include <stdio.h>

void volumetric_debug_collect(const volumetric_pass *p,
                              volumetric_debug_stats *out) {
    out->enabled   = p->enabled;
    out->active    = volumetric_pass_active(p);
    out->steps     = p->params.steps;
    out->scale     = p->params.scale;
    out->march_w   = p->target.w;
    out->march_h   = p->target.h;
    out->elevation = volumetric_sun_elevation(p->to_sun);
    out->strength  = p->profile.strength;
    out->g         = p->profile.medium.g;
    out->scatter   = p->profile.medium.scatter;
    out->extinct   = p->profile.medium.extinct;
    out->frame     = p->frame;
}

int volumetric_debug_format(const volumetric_debug_stats *s,
                            char *buf, size_t cap) {
    if (!buf || cap == 0) return 0;
    // one terse line. the state tag up front so it's grep-able in a log dump.
    const char *tag = s->active ? "on" : (s->enabled ? "idle" : "off");
    int n = snprintf(buf, cap,
                     "vol[%s] %dx%d x%d steps=%d elev=%.2f str=%.2f "
                     "g=%.2f s=%.3f e=%.3f",
                     tag, s->march_w, s->march_h, s->scale, s->steps,
                     s->elevation, s->strength,
                     s->g, s->scatter, s->extinct);
    // snprintf returns the would-be length; clamp so callers can trust it.
    if (n < 0) return 0;
    if ((size_t)n >= cap) return (int)(cap - 1);
    return n;
}

long volumetric_debug_sample_budget(const volumetric_debug_stats *s) {
    long pixels = (long)s->march_w * (long)s->march_h;
    return pixels * (long)s->steps;
}
