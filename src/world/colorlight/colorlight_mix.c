#include "colorlight_mix.h"

#include <math.h>

static uint8_t clamp255(float v) {
    if (v <= 0.0f) return 0;
    if (v >= 255.0f) return 255;
    return (uint8_t)(v + 0.5f);
}

colorlight_rgb colorlight_mix_saturate(colorlight_rgb c, float s) {
    // luma as the gray anchor, then lerp each channel away from it by s.
    float l = (float)colorlight_rgb_luma(c);
    colorlight_rgb o;
    o.r = clamp255(l + (c.r - l) * s);
    o.g = clamp255(l + (c.g - l) * s);
    o.b = clamp255(l + (c.b - l) * s);
    return o;
}

// cheap gamma without powf. for g<1 we blend toward sqrt (lift), for g>1 toward
// square (crush). good enough for light grading, nobody's measuring it.
static uint8_t gamma_chan(uint8_t v, float g) {
    float x = v / 255.0f;
    float lifted;
    if (g < 1.0f) {
        float sq = sqrtf(x);
        float t = 1.0f - g;            // 0..1, how far toward sqrt
        lifted = x + (sq - x) * t;
    } else if (g > 1.0f) {
        float sqd = x * x;
        float t = g - 1.0f;
        if (t > 1.0f) t = 1.0f;
        lifted = x + (sqd - x) * t;
    } else {
        lifted = x;
    }
    return clamp255(lifted * 255.0f);
}

colorlight_rgb colorlight_mix_gamma(colorlight_rgb c, float g) {
    colorlight_rgb o;
    o.r = gamma_chan(c.r, g);
    o.g = gamma_chan(c.g, g);
    o.b = gamma_chan(c.b, g);
    return o;
}

colorlight_rgb colorlight_mix_temperature(colorlight_rgb c, float t) {
    if (t > 1.0f) t = 1.0f;
    if (t < -1.0f) t = -1.0f;
    // +-48 at full swing, split across red and blue in opposite directions.
    float shift = t * 48.0f;
    colorlight_rgb o;
    o.r = clamp255(c.r + shift);
    o.g = c.g;
    o.b = clamp255(c.b - shift);
    return o;
}

colorlight_rgb colorlight_mix_grade(colorlight_rgb c) {
    // mild fixed grade: a little more saturated, shadows lifted slightly. tuned
    // by eye against torchlight on stone, adjust to taste.
    c = colorlight_mix_saturate(c, 1.12f);
    c = colorlight_mix_gamma(c, 0.9f);
    return c;
}
