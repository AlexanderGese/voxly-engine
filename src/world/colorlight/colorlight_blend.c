#include "colorlight_blend.h"
#include "colorlight_mix.h"
#include "../../config.h"

// module-static sun tint. white until the day/night code says otherwise.
static colorlight_rgb s_sun_tint = { 255, 255, 255 };

void colorlight_blend_set_sun_tint(colorlight_rgb tint) {
    s_sun_tint = tint;
}

colorlight_rgb colorlight_blend_sun_tint(void) {
    return s_sun_tint;
}

// scalar sunlight -> tinted rgb. level/15 of the current sun tint.
static colorlight_rgb sun_color(uint8_t sun) {
    if (sun > MAX_LIGHT) sun = MAX_LIGHT;
    if (sun == 0) return COLORLIGHT_BLACK;
    return colorlight_rgb_scale(s_sun_tint, (float)sun / (float)MAX_LIGHT);
}

colorlight_rgb colorlight_blend(uint8_t sun, colorlight_packed blk) {
    colorlight_rgb s = sun_color(sun);
    colorlight_rgb b = colorlight_packed_widen(blk);
    // brightest channel wins so daylight + torch doesn't over-saturate.
    return colorlight_rgb_max(s, b);
}

colorlight_rgb colorlight_blend_ambient(uint8_t sun, colorlight_packed blk, colorlight_rgb amb) {
    colorlight_rgb lit = colorlight_blend(sun, blk);
    // ambient is a floor, not additive. keeps deep caves from being a void
    // without washing out anywhere that's actually lit.
    lit = colorlight_rgb_max(lit, amb);
    // final grade so torchlight pops a bit. cheap, runs per sample.
    return colorlight_mix_grade(lit);
}

uint16_t colorlight_blend_to_565(colorlight_rgb c) {
    // 8->5/6/5 by dropping low bits. standard, nothing fancy.
    uint16_t r = (c.r >> 3) & 0x1F;
    uint16_t g = (c.g >> 2) & 0x3F;
    uint16_t b = (c.b >> 3) & 0x1F;
    return (uint16_t)((r << 11) | (g << 5) | b);
}
