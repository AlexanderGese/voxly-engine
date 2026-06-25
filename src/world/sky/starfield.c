#include "starfield.h"
#include "sky_math.h"
#include "sky_rand.h"

#include <math.h>

void voxl_sky_starfield_init(voxl_sky_starfield *sf, int count, unsigned seed) {
    if (count > VOXL_SKY_STAR_MAX) count = VOXL_SKY_STAR_MAX;
    if (count < 0) count = 0;
    sf->count = count;

    voxl_sky_rand rng;
    voxl_sky_rand_seed(&rng, (uint32_t)seed);

    for (int i = 0; i < count; i++) {
        voxl_sky_star *s = &sf->stars[i];

        // sample a roughly uniform point on the upper hemisphere.
        // pick z (up) in [0,1], azimuth around, then build the dir.
        float up  = voxl_sky_rand_f(&rng);                  // 0..1 -> bias up
        float az  = voxl_sky_rand_range(&rng, 0.0f, VOXL_SKY_TAU);
        float r   = sqrtf(1.0f - up * up);                  // radius in xz plane
        s->dir.x = r * cosf(az);
        s->dir.y = up;
        s->dir.z = r * sinf(az);

        s->base    = voxl_sky_rand_range(&rng, 0.3f, 1.0f);
        s->twinkle = voxl_sky_rand_range(&rng, 0.5f, 3.0f);
    }
}

float voxl_sky_star_visibility(float hour) {
    hour = voxl_sky_wrap24(hour);
    // visible after dusk (~20) and before dawn (~5), fading at the edges.
    if (hour >= 21.0f || hour <= 4.0f) return 1.0f;
    if (hour > 4.0f && hour < 6.0f)   return voxl_sky_smooth(6.0f, 4.0f, hour);
    if (hour > 19.0f && hour < 21.0f) return voxl_sky_smooth(19.0f, 21.0f, hour);
    return 0.0f;
}

float voxl_sky_star_brightness(const voxl_sky_star *s, float hour, float time_s) {
    float vis = voxl_sky_star_visibility(hour);
    if (vis <= 0.0f) return 0.0f;
    // twinkle: a slow sine offset per star so they don't pulse in unison.
    float phase = time_s * s->twinkle + s->base * 10.0f;
    float tw = 0.75f + 0.25f * sinf(phase);
    return voxl_sky_clampf(vis * s->base * tw, 0.0f, 1.0f);
}
