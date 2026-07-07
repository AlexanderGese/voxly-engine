#ifndef RENDER_FXAA_HISTORY_H
#define RENDER_FXAA_HISTORY_H

// a small ring of recent edge-fraction samples (from fxaa_debug_stats). it
// smooths the per-frame number for the overlay and feeds an optional auto
// quality heuristic: scenes that are mostly flat (few edges) dont need the
// expensive search, busy ones do. nothing here touches gl.

#define FXAA_HISTORY_LEN 32

typedef struct {
    float samples[FXAA_HISTORY_LEN];
    int   count;        // valid entries so far (caps at LEN)
    int   head;         // next write index, ring
    float running_sum;  // kept incrementally so avg() is O(1)
} fxaa_history;

// zero it out.
void  fxaa_history_reset(fxaa_history *h);

// push one edge-fraction sample in [0,1]. evicts the oldest once full.
void  fxaa_history_push(fxaa_history *h, float edge_frac);

// moving average of the buffered samples. 0 if empty.
float fxaa_history_avg(const fxaa_history *h);

// peak edge fraction currently in the buffer. 0 if empty.
float fxaa_history_peak(const fxaa_history *h);

// suggest a FXAA_QUALITY_* preset from the smoothed edge density. mostly-flat
// frames get a cheaper preset; edge-dense frames get the expensive one. the
// thresholds have hysteresis baked in via the cur arg so it doesnt flap.
int   fxaa_history_suggest_quality(const fxaa_history *h, int cur);

#endif
