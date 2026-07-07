#include "fxaa_quality.h"

#include <stddef.h>

// the step tables. transcribed from the classic fxaa quality presets, give or
// take. low/medium keep the count down; high/extreme add the long tail of
// coarse steps that lets shallow near-horizontal edges resolve.

static const float steps_low[] = {
    1.0f, 1.5f, 2.0f, 4.0f,
};

static const float steps_medium[] = {
    1.0f, 1.5f, 2.0f, 2.0f, 4.0f, 8.0f,
};

static const float steps_high[] = {
    1.0f, 1.5f, 2.0f, 2.0f, 2.0f, 4.0f, 8.0f,
};

static const float steps_extreme[] = {
    1.0f, 1.0f, 1.0f, 1.5f, 2.0f, 2.0f, 2.0f, 2.0f, 4.0f, 8.0f,
};

static const fxaa_quality presets[FXAA_QUALITY_COUNT] = {
    { (int)(sizeof steps_low     / sizeof steps_low[0]),     steps_low,     8.0f  },
    { (int)(sizeof steps_medium  / sizeof steps_medium[0]),  steps_medium,  8.0f  },
    { (int)(sizeof steps_high    / sizeof steps_high[0]),    steps_high,    8.0f  },
    { (int)(sizeof steps_extreme / sizeof steps_extreme[0]), steps_extreme, 12.0f },
};

static const char *names[FXAA_QUALITY_COUNT] = {
    "low", "medium", "high", "extreme",
};

const fxaa_quality *fxaa_quality_get(int preset) {
    if (preset < 0) preset = 0;
    if (preset >= FXAA_QUALITY_COUNT) preset = FXAA_QUALITY_COUNT - 1;
    return &presets[preset];
}

float fxaa_quality_reach(const fxaa_quality *q) {
    if (!q || !q->steps) return 0.0f;
    float sum = 0.0f;
    for (int i = 0; i < q->step_count; i++)
        sum += q->steps[i];
    // the search runs symmetrically in both directions, so the real reach is
    // double the one-sided sum. callers that want clamp margins want this.
    return sum * 2.0f;
}

const char *fxaa_quality_name(int preset) {
    if (preset < 0) preset = 0;
    if (preset >= FXAA_QUALITY_COUNT) preset = FXAA_QUALITY_COUNT - 1;
    return names[preset];
}
