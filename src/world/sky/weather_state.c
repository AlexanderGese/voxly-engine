#include "weather_state.h"
#include "sky_math.h"

// how fast intensity chases its target, per second.
#define VOXL_SKY_WEATHER_RAMP 0.25f

static float roll_duration(voxl_sky_rand *rng, voxl_sky_weather_kind k) {
    // clear spells last longer than bad weather, roughly.
    if (k == VOXL_SKY_CLEAR) return voxl_sky_rand_range(rng, 90.0f, 240.0f);
    if (k == VOXL_SKY_STORM) return voxl_sky_rand_range(rng, 30.0f, 80.0f);
    return voxl_sky_rand_range(rng, 60.0f, 150.0f);
}

static void pick_next(voxl_sky_weather *w) {
    w->prev  = w->kind;
    w->blend = 0.0f;

    // weighted-ish roll. mostly clear, sometimes rain/snow, rarely storm.
    int r = voxl_sky_rand_int(&w->rng, 0, 99);
    voxl_sky_weather_kind k;
    if      (r < 50) k = VOXL_SKY_CLEAR;
    else if (r < 75) k = VOXL_SKY_RAIN;
    else if (r < 92) k = VOXL_SKY_SNOW;
    else             k = VOXL_SKY_STORM;

    w->kind     = k;
    w->target   = (k == VOXL_SKY_CLEAR) ? 0.0f
                : (k == VOXL_SKY_STORM) ? voxl_sky_rand_range(&w->rng, 0.7f, 1.0f)
                : voxl_sky_rand_range(&w->rng, 0.3f, 0.8f);
    w->timer    = 0.0f;
    w->duration = roll_duration(&w->rng, k);
}

void voxl_sky_weather_init(voxl_sky_weather *w, unsigned seed) {
    voxl_sky_rand_seed(&w->rng, (uint32_t)seed);
    w->kind = w->prev = VOXL_SKY_CLEAR;
    w->intensity = 0.0f;
    w->target = 0.0f;
    w->timer = 0.0f;
    w->blend = 1.0f;
    w->duration = roll_duration(&w->rng, VOXL_SKY_CLEAR);
}

void voxl_sky_weather_force(voxl_sky_weather *w, voxl_sky_weather_kind k, float target) {
    w->prev = w->kind;
    w->kind = k;
    w->target = voxl_sky_clampf(target, 0.0f, 1.0f);
    w->timer = 0.0f;
    w->blend = 0.0f;
    w->duration = roll_duration(&w->rng, k);
}

void voxl_sky_weather_update(voxl_sky_weather *w, float dt) {
    if (dt < 0.0f) dt = 0.0f;
    w->timer += dt;
    if (w->timer >= w->duration) {
        pick_next(w);
    }

    // cross-fade tracker (purely informational for blending visuals).
    w->blend = voxl_sky_clampf(w->blend + dt * 0.5f, 0.0f, 1.0f);

    // chase target intensity.
    float step = VOXL_SKY_WEATHER_RAMP * dt;
    if (w->intensity < w->target) {
        w->intensity = voxl_sky_clampf(w->intensity + step, 0.0f, w->target);
    } else if (w->intensity > w->target) {
        w->intensity = voxl_sky_clampf(w->intensity - step, w->target, 1.0f);
    }
}

float voxl_sky_weather_wetness(const voxl_sky_weather *w) {
    // snow contributes less "wetness" (overcast) than rain/storm.
    float k = w->intensity;
    if (w->kind == VOXL_SKY_SNOW) k *= 0.7f;
    return voxl_sky_clampf(k, 0.0f, 1.0f);
}

int voxl_sky_weather_is_precip(const voxl_sky_weather *w) {
    if (w->kind == VOXL_SKY_CLEAR) return 0;
    return w->intensity > 0.02f;
}

const char *voxl_sky_weather_name(voxl_sky_weather_kind k) {
    switch (k) {
    case VOXL_SKY_CLEAR: return "clear";
    case VOXL_SKY_RAIN:  return "rain";
    case VOXL_SKY_SNOW:  return "snow";
    case VOXL_SKY_STORM: return "storm";
    default:             return "?";
    }
}
