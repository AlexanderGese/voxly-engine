#include "sky_color.h"
#include "sky_gradient.h"
#include "sky_math.h"

vec3 voxl_sky_fog_color(float hour) {
    // horizon-ish color works well as fog. sample the gradient low.
    return voxl_sky_gradient_at(hour, 0.15f);
}

float voxl_sky_sun_brightness(float hour) {
    hour = voxl_sky_wrap24(hour);
    // ramp up after 5, full by 8, hold til 18, ramp down to 0 by 21.
    if (hour < 5.0f || hour > 21.0f) return 0.0f;
    if (hour < 8.0f)  return voxl_sky_smooth(5.0f, 8.0f, hour);
    if (hour > 18.0f) return voxl_sky_smooth(21.0f, 18.0f, hour);
    return 1.0f;
}

vec3 voxl_sky_sun_tint(float hour) {
    hour = voxl_sky_wrap24(hour);
    // three anchor colors: night moonlight, golden hour, midday white.
    vec3 moon   = {0.35f, 0.42f, 0.65f};
    vec3 golden = {1.00f, 0.62f, 0.34f};
    vec3 noon   = {1.00f, 0.97f, 0.90f};

    float b = voxl_sky_sun_brightness(hour);
    if (b <= 0.0f) return moon;

    // how close are we to the horizon-grazing hours (golden hour)?
    float morning = voxl_sky_smooth(8.0f, 6.0f, hour);   // 1 near 6, 0 by 8
    float evening = voxl_sky_smooth(16.0f, 19.0f, hour);  // 0 til 16, 1 by 19
    float golden_amt = voxl_sky_clampf(morning + evening, 0.0f, 1.0f);

    vec3 day = voxl_sky_mix3(noon, golden, golden_amt);
    return voxl_sky_mix3(moon, day, b);
}

vec3 voxl_sky_apply_overcast(vec3 color, float wetness) {
    wetness = voxl_sky_clampf(wetness, 0.0f, 1.0f);
    // grey we drift toward as it gets stormier.
    vec3 overcast = {0.42f, 0.44f, 0.47f};
    // luminance-ish average for the desaturation pull.
    float lum = color.x * 0.30f + color.y * 0.59f + color.z * 0.11f;
    vec3 grey = {lum, lum, lum};
    // first desaturate a bit, then darken toward overcast.
    vec3 desat = voxl_sky_mix3(color, grey, wetness * 0.5f);
    return voxl_sky_mix3(desat, overcast, wetness * 0.6f);
}
