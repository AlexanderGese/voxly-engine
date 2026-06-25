#include "particle_emitter_weather.h"

void voxl_sky_emitter_init(voxl_sky_weather_emitter *e, unsigned seed,
                           float box, float top) {
    voxl_sky_rain_init(&e->rain, seed, box, top);
    voxl_sky_snow_init(&e->snow, seed ^ 0x5a5a5a5au, box, top);
    e->mode = VOXL_SKY_EMIT_NONE;
}

void voxl_sky_emitter_update(voxl_sky_weather_emitter *e,
                             const voxl_sky_weather *w,
                             float dt, vec3 center) {
    float wet = w->intensity;

    // decide what's falling. storm rains too, just harder.
    voxl_sky_emit_mode want = VOXL_SKY_EMIT_NONE;
    if (w->kind == VOXL_SKY_RAIN || w->kind == VOXL_SKY_STORM) {
        want = VOXL_SKY_EMIT_RAIN;
    } else if (w->kind == VOXL_SKY_SNOW) {
        want = VOXL_SKY_EMIT_SNOW;
    }
    e->mode = want;

    // feed intensity to the active system, and drain the inactive one.
    if (want == VOXL_SKY_EMIT_RAIN) {
        voxl_sky_rain_set_intensity(&e->rain, wet);
        voxl_sky_snow_set_intensity(&e->snow, 0.0f);
    } else if (want == VOXL_SKY_EMIT_SNOW) {
        voxl_sky_snow_set_intensity(&e->snow, wet);
        voxl_sky_rain_set_intensity(&e->rain, 0.0f);
    } else {
        voxl_sky_rain_set_intensity(&e->rain, 0.0f);
        voxl_sky_snow_set_intensity(&e->snow, 0.0f);
    }

    // always update both so live particles finish falling on a transition.
    voxl_sky_rain_update(&e->rain, dt, center);
    voxl_sky_snow_update(&e->snow, dt, center);
}

int voxl_sky_emitter_live_count(const voxl_sky_weather_emitter *e) {
    return e->rain.count + e->snow.count;
}
