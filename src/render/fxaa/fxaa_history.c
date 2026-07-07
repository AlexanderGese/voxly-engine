#include "fxaa_history.h"
#include "fxaa_config.h"

#include <stddef.h>

void fxaa_history_reset(fxaa_history *h) {
    for (int i = 0; i < FXAA_HISTORY_LEN; i++) h->samples[i] = 0.0f;
    h->count = 0;
    h->head = 0;
    h->running_sum = 0.0f;
}

void fxaa_history_push(fxaa_history *h, float edge_frac) {
    if (edge_frac < 0.0f) edge_frac = 0.0f;
    if (edge_frac > 1.0f) edge_frac = 1.0f;

    // subtract whatever we're about to overwrite, then add the new sample.
    // keeps running_sum exact without a full re-scan each push.
    h->running_sum -= h->samples[h->head];
    h->samples[h->head] = edge_frac;
    h->running_sum += edge_frac;

    h->head = (h->head + 1) % FXAA_HISTORY_LEN;
    if (h->count < FXAA_HISTORY_LEN) h->count++;
}

float fxaa_history_avg(const fxaa_history *h) {
    if (h->count <= 0) return 0.0f;
    return h->running_sum / (float)h->count;
}

float fxaa_history_peak(const fxaa_history *h) {
    float mx = 0.0f;
    int n = h->count;
    for (int i = 0; i < n; i++)
        if (h->samples[i] > mx) mx = h->samples[i];
    return mx;
}

int fxaa_history_suggest_quality(const fxaa_history *h, int cur) {
    if (cur < 0) cur = 0;
    if (cur >= FXAA_QUALITY_COUNT) cur = FXAA_QUALITY_COUNT - 1;
    if (h->count < FXAA_HISTORY_LEN / 2)
        return cur;   // not enough history yet, dont jump around

    float avg = fxaa_history_avg(h);

    // hysteresis: only step one level at a time, and use wider bands than the
    // step-down would imply so we dont oscillate on a borderline scene. these
    // edge-fraction bands are eyeballed — flat menus sit near 0.02, a busy
    // foliage scene pushes past 0.15.
    if (avg < 0.03f && cur > FXAA_QUALITY_LOW)
        return cur - 1;
    if (avg > 0.10f && cur < FXAA_QUALITY_EXTREME)
        return cur + 1;
    return cur;
}
