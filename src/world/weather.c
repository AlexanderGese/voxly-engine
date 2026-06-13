#include "weather.h"
#include "../math/rng.h"

static rng wrng;

void weather_init(weather *w) {
    rng_init(&wrng, 0xbadcafe);
    w->state = WEATHER_CLEAR;
    w->intensity = 0;
    w->timer = 0;
    w->next_change = 120.0f + rng_frange(&wrng, -60, 120);
}

void weather_update(weather *w, float dt) {
    w->timer += dt;
    if (w->timer >= w->next_change) {
        w->timer = 0;
        w->next_change = 90.0f + rng_frange(&wrng, -30, 240);
        int r = rng_range(&wrng, 0, 4);
        if (r == 0) w->state = WEATHER_RAIN;
        else if (r == 1) w->state = WEATHER_SNOW;
        else w->state = WEATHER_CLEAR;
        w->intensity = w->state == WEATHER_CLEAR ? 0 : rng_frange(&wrng, 0.3f, 1.0f);
    }
}

const char *weather_name(weather_state s) {
    switch (s) {
    case WEATHER_CLEAR: return "clear";
    case WEATHER_RAIN:  return "rain";
    case WEATHER_SNOW:  return "snow";
    }
    return "?";
}
