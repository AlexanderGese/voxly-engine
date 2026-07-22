#include "vol_medium.h"
#include "vol_config.h"

#include <math.h>

static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

float volumetric_sun_elevation(vec3 to_sun) {
    float len = vec3_length(to_sun);
    if (len < 1e-6f) return -1.0f;        // no sun direction -> treat as night
    return clampf(to_sun.y / len, -1.0f, 1.0f);
}

float volumetric_horizon_ramp(float elevation) {
    // below ground: nothing. the sun isn't lighting the air column.
    if (elevation <= 0.0f) return 0.0f;

    // a smooth bump that peaks low. we want the strongest shafts when the sun
    // grazes the horizon (long slant paths through the atmosphere) and a gentle
    // taper toward noon where the column is short and the light is overhead.
    // peak sits around elevation ~0.12 (a few degrees up).
    const float peak = 0.12f;
    float x = elevation / peak;
    // a quick-rise, slow-fall lobe. rises like x near 0, decays exponentially.
    float ramp = x * expf(1.0f - x);
    return clampf(ramp, 0.0f, 1.0f);
}

void volumetric_medium_profile_from_sun(volumetric_medium_profile *p,
                                        vec3 to_sun,
                                        float base_g,
                                        float base_scatter,
                                        float base_extinct) {
    float elev = volumetric_sun_elevation(to_sun);
    float ramp = volumetric_horizon_ramp(elev);

    // forward scattering gets stronger near the horizon — the tight halo around
    // a low sun is exactly high-g hg. lerp g up toward the cap as the sun sets.
    float g = lerpf(base_g, VOL_G_MAX, 1.0f - clampf(elev, 0.0f, 1.0f));

    // thicker air near the horizon: scale both coefficients up with the ramp,
    // but never below the base so high noon still has a whisper of haze.
    float thick = 1.0f + 1.5f * ramp;
    float scatter = base_scatter * thick;
    float extinct = base_extinct * thick;

    volumetric_medium_init(&p->medium, g, scatter, extinct);

    // tint: warm/red the light as the sun drops. at noon it's near-white; near
    // the horizon we pull green and blue down so shafts go gold then red.
    float warmth = 1.0f - clampf(elev * 3.0f, 0.0f, 1.0f);  // 1 at horizon
    p->tint = vec3_new(1.0f,
                       lerpf(1.0f, 0.62f, warmth),
                       lerpf(1.0f, 0.34f, warmth));

    // overall strength follows the ramp but with a small floor while the sun is
    // up, so the effect doesn't pop on/off as elevation crosses the peak.
    p->strength = (elev > 0.0f) ? clampf(0.15f + ramp, 0.0f, 1.0f) : 0.0f;
}
