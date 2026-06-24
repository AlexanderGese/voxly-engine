#include "fog_density.h"
#include "sky_math.h"
#include "sky_color.h"

#include <math.h>

voxl_sky_fog voxl_sky_fog_params(float hour, float wetness, float view_dist) {
    wetness = voxl_sky_clampf(wetness, 0.0f, 1.0f);
    if (view_dist < 1.0f) view_dist = 1.0f;

    // at night the world feels a touch hazier; weather pulls fog way in.
    float day = voxl_sky_sun_brightness(hour);
    float night_haze = voxl_sky_lerpf(0.10f, 0.0f, day);   // up to 10% closer
    float pull = voxl_sky_clampf(wetness * 0.6f + night_haze, 0.0f, 0.75f);

    voxl_sky_fog f;
    // linear band: starts ~40% out, ends at the far plane, both pulled in.
    f.end   = view_dist * (1.0f - pull);
    f.start = f.end * voxl_sky_lerpf(0.45f, 0.15f, wetness);
    if (f.start < 1.0f) f.start = 1.0f;
    if (f.end <= f.start) f.end = f.start + 1.0f;

    // matching exp2 density, tuned so factor ~0.5 around the linear midpoint.
    float mid = (f.start + f.end) * 0.5f;
    f.density = 1.0f / mid;
    return f;
}

float voxl_sky_fog_factor(float distance, float density) {
    if (distance <= 0.0f) return 1.0f;
    float fd = density * distance;
    float f = expf(-(fd * fd));   // exp2 fog
    return voxl_sky_clampf(f, 0.0f, 1.0f);
}
