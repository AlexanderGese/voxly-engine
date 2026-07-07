#ifndef RENDER_FXAA_DEBUG_H
#define RENDER_FXAA_DEBUG_H

#include "fxaa.h"

// debug helpers: readback stats off the prepass target and a one-line
// formatter for the on-screen overlay. these are diagnostic only; nothing in
// the render path depends on them.

typedef struct {
    int   pixels;          // texels sampled
    float avg_luma;        // mean of the alpha (luma) channel, 0..1
    float min_luma;
    float max_luma;
    float edge_frac;       // fraction of texels whose local contrast clears
                           // the current edge threshold (an edge estimate)
} fxaa_stats;

// estimate edge fraction + luma stats by walking a cpu copy of the target's
// luma channel. `luma` is w*h floats in [0,1]. pure cpu, no gl.
void fxaa_debug_stats_cpu(const float *luma, int w, int h,
                          float edge_threshold, fxaa_stats *out);

// readback the live prepass target and compute stats. returns 0 if fxaa isnt
// in its gpu path (nothing to read). does a glReadPixels so it's not free —
// call it once a second from the debug ui, not every frame.
int  fxaa_debug_stats(const fxaa *f, fxaa_stats *out);

// format stats into buf for the overlay.
void fxaa_debug_format(const fxaa *f, const fxaa_stats *st,
                       char *buf, int buflen);

#endif
