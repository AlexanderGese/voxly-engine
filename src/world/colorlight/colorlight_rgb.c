#include "colorlight_rgb.h"

static uint8_t sat_add(int a, int b) {
    int s = a + b;
    return (uint8_t)(s > 255 ? 255 : s);
}

static uint8_t clamp255(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

colorlight_rgb colorlight_rgb_new(uint8_t r, uint8_t g, uint8_t b) {
    colorlight_rgb c = { r, g, b };
    return c;
}

colorlight_rgb colorlight_rgb_max(colorlight_rgb a, colorlight_rgb b) {
    colorlight_rgb c;
    c.r = a.r > b.r ? a.r : b.r;
    c.g = a.g > b.g ? a.g : b.g;
    c.b = a.b > b.b ? a.b : b.b;
    return c;
}

colorlight_rgb colorlight_rgb_add(colorlight_rgb a, colorlight_rgb b) {
    colorlight_rgb c;
    c.r = sat_add(a.r, b.r);
    c.g = sat_add(a.g, b.g);
    c.b = sat_add(a.b, b.b);
    return c;
}

colorlight_rgb colorlight_rgb_scale(colorlight_rgb a, float s) {
    if (s <= 0.0f) return COLORLIGHT_BLACK;
    if (s >= 1.0f) return a;
    colorlight_rgb c;
    // +0.5 to round instead of truncate. matters for low channels.
    c.r = clamp255((int)(a.r * s + 0.5f));
    c.g = clamp255((int)(a.g * s + 0.5f));
    c.b = clamp255((int)(a.b * s + 0.5f));
    return c;
}

colorlight_rgb colorlight_rgb_mul(colorlight_rgb a, colorlight_rgb b) {
    colorlight_rgb c;
    c.r = (uint8_t)((a.r * b.r) / 255);
    c.g = (uint8_t)((a.g * b.g) / 255);
    c.b = (uint8_t)((a.b * b.b) / 255);
    return c;
}

colorlight_rgb colorlight_rgb_lerp(colorlight_rgb a, colorlight_rgb b, float t) {
    if (t <= 0.0f) return a;
    if (t >= 1.0f) return b;
    colorlight_rgb c;
    c.r = clamp255((int)(a.r + (b.r - a.r) * t + 0.5f));
    c.g = clamp255((int)(a.g + (b.g - a.g) * t + 0.5f));
    c.b = clamp255((int)(a.b + (b.b - a.b) * t + 0.5f));
    return c;
}

uint8_t colorlight_rgb_peak(colorlight_rgb a) {
    uint8_t m = a.r;
    if (a.g > m) m = a.g;
    if (a.b > m) m = a.b;
    return m;
}

uint8_t colorlight_rgb_luma(colorlight_rgb a) {
    // rec601: 0.299 / 0.587 / 0.114, scaled to integer weights summing 256.
    int l = (77 * a.r + 150 * a.g + 29 * a.b) >> 8;
    return clamp255(l);
}

int colorlight_rgb_equal(colorlight_rgb a, colorlight_rgb b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

int colorlight_rgb_is_black(colorlight_rgb a) {
    return a.r == 0 && a.g == 0 && a.b == 0;
}
