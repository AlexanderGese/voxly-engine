#ifndef RENDER_SSAO_DEBUG_H
#define RENDER_SSAO_DEBUG_H
#include "ssao_pass.h"
#include "ssao_settings.h"
// debugging + glue helpers for ssao. push the user-facing settings onto a
// live pass, read back the occlusion buffer for a sanity overlay, and grab
// some quick stats so you can tell at a glance whether ssao is doing anything
// or just eating frame time.
// apply settings to a pass. handles kernel resize/reseed and scale changes
// (which require resizing the occlusion buffer). returns 1 if the pass needed
int  ssaox_debug_apply(ssaox_pass *p, const ssaox_settings *s);
typedef struct {
    float min_occl;     // darkest pixel (most occluded), 0..1
    float max_occl;     // brightest pixel (least occluded)
    float avg_occl;     // mean across the buffer
    float dark_frac;    // fraction of pixels below 0.5 (visibly occluded)
    int   pixels;       // total sampled
} ssaox_stats;
int  ssaox_debug_stats(const ssaox_pass *p, ssaox_stats *out);
void ssaox_debug_stats_cpu(const float *occl, int w, int h, ssaox_stats *out);
void ssaox_debug_format(const ssaox_stats *st, char *buf, int buflen);
#endif
