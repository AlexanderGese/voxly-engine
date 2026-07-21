#ifndef RENDER_VOLUMETRIC_DEBUG_H
#define RENDER_VOLUMETRIC_DEBUG_H
#include "vol_pass.h"
#include <stddef.h>
// read-only introspection for the f3-style debug overlay. no gl here — just
// pulls numbers out of the pass and formats a one-liner. handy when you're
// hand-tuning the scattering at sunset and want to see what the profile baked.
typedef struct {
    int    enabled;       // pass.enabled
    int    active;        // will it draw this frame
    int    steps;
    int    scale;
    int    march_w, march_h;   // reduced resolution
    float  elevation;     // sun elevation [-1,1]
    float  strength;      // profile strength
    float  g, scatter, extinct;
    unsigned frame;
} volumetric_debug_stats;
// snapshot the pass into a stats struct.
void volumetric_debug_collect(const volumetric_pass *p,
                              volumetric_debug_stats *out);
// format a compact human line into buf (e.g. for the hud). returns the number
// of chars written (excluding the nul), like snprintf. never overflows cap.
int  volumetric_debug_format(const volumetric_debug_stats *s,
                             char *buf, size_t cap);
// rough per-frame march sample budget: march_w * march_h * steps. lets the
long volumetric_debug_sample_budget(const volumetric_debug_stats *s);
#endif
