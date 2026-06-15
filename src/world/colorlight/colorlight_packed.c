#include "colorlight_packed.h"

static uint8_t clampl(uint8_t v) { return v > 15 ? 15 : v; }

uint8_t colorlight_packed_r(colorlight_packed p) {
    return (p >> COLORLIGHT_R_SHIFT) & COLORLIGHT_CHAN_MASK;
}

uint8_t colorlight_packed_g(colorlight_packed p) {
    return (p >> COLORLIGHT_G_SHIFT) & COLORLIGHT_CHAN_MASK;
}

uint8_t colorlight_packed_b(colorlight_packed p) {
    return (p >> COLORLIGHT_B_SHIFT) & COLORLIGHT_CHAN_MASK;
}

uint8_t colorlight_packed_chan(colorlight_packed p, int chan) {
    switch (chan) {
        case 0: return colorlight_packed_r(p);
        case 1: return colorlight_packed_g(p);
        case 2: return colorlight_packed_b(p);
        default: return 0;
    }
}

colorlight_packed colorlight_packed_set_r(colorlight_packed p, uint8_t v) {
    v = clampl(v);
    p &= ~(COLORLIGHT_CHAN_MASK << COLORLIGHT_R_SHIFT);
    return p | ((colorlight_packed)v << COLORLIGHT_R_SHIFT);
}

colorlight_packed colorlight_packed_set_g(colorlight_packed p, uint8_t v) {
    v = clampl(v);
    p &= ~(COLORLIGHT_CHAN_MASK << COLORLIGHT_G_SHIFT);
    return p | ((colorlight_packed)v << COLORLIGHT_G_SHIFT);
}

colorlight_packed colorlight_packed_set_b(colorlight_packed p, uint8_t v) {
    v = clampl(v);
    p &= ~(COLORLIGHT_CHAN_MASK << COLORLIGHT_B_SHIFT);
    return p | ((colorlight_packed)v << COLORLIGHT_B_SHIFT);
}

colorlight_packed colorlight_packed_set_chan(colorlight_packed p, int chan, uint8_t v) {
    switch (chan) {
        case 0: return colorlight_packed_set_r(p, v);
        case 1: return colorlight_packed_set_g(p, v);
        case 2: return colorlight_packed_set_b(p, v);
        default: return p;
    }
}

colorlight_packed colorlight_packed_make(uint8_t r, uint8_t g, uint8_t b) {
    colorlight_packed p = 0;
    p |= (colorlight_packed)clampl(r) << COLORLIGHT_R_SHIFT;
    p |= (colorlight_packed)clampl(g) << COLORLIGHT_G_SHIFT;
    p |= (colorlight_packed)clampl(b) << COLORLIGHT_B_SHIFT;
    return p;
}

colorlight_rgb colorlight_packed_widen(colorlight_packed p) {
    // 15 levels * 17 = 255. clean mapping, no rounding needed going up.
    colorlight_rgb c;
    c.r = (uint8_t)(colorlight_packed_r(p) * 17);
    c.g = (uint8_t)(colorlight_packed_g(p) * 17);
    c.b = (uint8_t)(colorlight_packed_b(p) * 17);
    return c;
}

colorlight_packed colorlight_packed_narrow(colorlight_rgb c) {
    // round to nearest level: (v + 8) / 17.
    uint8_t r = (uint8_t)((c.r + 8) / 17);
    uint8_t g = (uint8_t)((c.g + 8) / 17);
    uint8_t b = (uint8_t)((c.b + 8) / 17);
    return colorlight_packed_make(r, g, b);
}

uint8_t colorlight_packed_peak(colorlight_packed p) {
    uint8_t r = colorlight_packed_r(p);
    uint8_t g = colorlight_packed_g(p);
    uint8_t b = colorlight_packed_b(p);
    uint8_t m = r;
    if (g > m) m = g;
    if (b > m) m = b;
    return m;
}
