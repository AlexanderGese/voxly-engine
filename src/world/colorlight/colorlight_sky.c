#include "colorlight_sky.h"
#include "colorlight_blend.h"

static uint8_t f2b(float v) {
    if (v <= 0.0f) return 0;
    if (v >= 1.0f) return 255;
    return (uint8_t)(v * 255.0f + 0.5f);
}

colorlight_rgb colorlight_sky_sun_tint(const time_of_day *t) {
    // tod gives us a sun color in [0,1] plus a separate strength. multiply them
    // so the tint at the grid is "how much white-ish light the sky is throwing".
    float s = t->sun_strength;
    if (s < 0.0f) s = 0.0f;
    if (s > 1.0f) s = 1.0f;

    colorlight_rgb c;
    c.r = f2b(t->sun_r * s);
    c.g = f2b(t->sun_g * s);
    c.b = f2b(t->sun_b * s);

    // never hand blend a pure-black tint while the sun's technically up; a
    // 1-bit floor keeps daylit faces from snapping to the ambient color at the
    // exact horizon crossing. cosmetic but the pop is ugly.
    if (s > 0.02f) {
        if (c.r == 0) c.r = 1;
        if (c.g == 0) c.g = 1;
        if (c.b == 0) c.b = 1;
    }
    return c;
}

void colorlight_sky_update(const time_of_day *t) {
    colorlight_blend_set_sun_tint(colorlight_sky_sun_tint(t));
}

colorlight_rgb colorlight_sky_night_ambient(const time_of_day *t) {
    // base cave-dark, lifted slightly and pushed blue when the sun's gone.
    float night = 1.0f - t->sun_strength;
    if (night < 0.0f) night = 0.0f;
    if (night > 1.0f) night = 1.0f;

    colorlight_rgb amb;
    amb.r = (uint8_t)(4 + 4 * night);
    amb.g = (uint8_t)(5 + 5 * night);
    amb.b = (uint8_t)(8 + 12 * night);
    return amb;
}
