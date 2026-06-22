#include "oregen_curve.h"

int oregen_curve_in_band(int y, int y_min, int y_max) {
    return (y >= y_min && y <= y_max);
}

float oregen_curve_t(int y, int y_min, int y_max) {
    if (y_max <= y_min) return 0.0f;     // degenerate band, treat as floor
    if (y <= y_min) return 0.0f;
    if (y >= y_max) return 1.0f;
    return (float)(y - y_min) / (float)(y_max - y_min);
}

float oregen_curve_eval(oregen_curve curve, float t) {
    // clamp so callers can be sloppy
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    switch (curve) {
        case OREGEN_CURVE_UNIFORM:
            return 1.0f;

        case OREGEN_CURVE_TRIANGLE: {
            // tent: 0 at edges, 1 in the middle.
            float d = t < 0.5f ? t : (1.0f - t);
            return d * 2.0f;
        }

        case OREGEN_CURVE_BOTTOM:
            // densest at the floor. ease so it isnt a hard linear ramp.
            // (1-t)^2 keeps a fat base and a thin tail near the ceiling.
            return (1.0f - t) * (1.0f - t);

        case OREGEN_CURVE_TOP:
            return t * t;

        default:
            return 1.0f;
    }
}

float oregen_curve_weight(const oregen_ore *ore, int y) {
    if (!ore) return 0.0f;
    if (!oregen_curve_in_band(y, ore->y_min, ore->y_max)) return 0.0f;
    float t = oregen_curve_t(y, ore->y_min, ore->y_max);
    return oregen_curve_eval(ore->curve, t);
}

float oregen_curve_band_mean(const oregen_ore *ore) {
    if (!ore) return 0.0f;
    int lo = ore->y_min, hi = ore->y_max;
    if (hi < lo) return 0.0f;

    int n = hi - lo + 1;
    if (n <= 1) return oregen_curve_eval(ore->curve, 0.0f);

    // the tent and the eased curves all have closed forms but sampling is
    // simpler and the band is small. accumulate and average.
    double acc = 0.0;
    for (int y = lo; y <= hi; y++) {
        float t = oregen_curve_t(y, lo, hi);
        acc += oregen_curve_eval(ore->curve, t);
    }
    return (float)(acc / (double)n);
}
