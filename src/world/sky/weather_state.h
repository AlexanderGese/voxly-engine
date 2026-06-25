#ifndef WORLD_SKY_WEATHER_STATE_H
#define WORLD_SKY_WEATHER_STATE_H
// a small weather state machine: clear / rain / snow / storm. it ticks a
// timer and rolls a new state when the timer runs out. intensity ramps in
// and out smoothly so transitions aren't a hard cut.
//
// this is a parallel, self-contained system to the engine's world/weather.c
// (i didn't want to touch that one) with a richer set of states + ramping.
#include "sky_rand.h"
typedef enum {
    VOXL_SKY_CLEAR = 0,
    VOXL_SKY_RAIN,
    VOXL_SKY_SNOW,
    VOXL_SKY_STORM,
    VOXL_SKY_WEATHER_COUNT
} voxl_sky_weather_kind;
typedef struct {
    voxl_sky_weather_kind kind;     // what we're heading toward
    voxl_sky_weather_kind prev;     // what we're fading out of
    float  intensity;               // current 0..1 (ramped)
    float  target;                  // target intensity for `kind`
    float  timer;                   // seconds in current state
    float  duration;                // when to roll next
    float  blend;                   // 0..1 cross-fade from prev->kind
    voxl_sky_rand rng;
} voxl_sky_weather;
void voxl_sky_weather_init(voxl_sky_weather *w, unsigned seed);
void voxl_sky_weather_update(voxl_sky_weather *w, float dt);
// force a state immediately (e.g. a command or trigger).
void voxl_sky_weather_force(voxl_sky_weather *w, voxl_sky_weather_kind k, float target);
// 0..1 "wetness" used by sky color / fog / lighting.
float voxl_sky_weather_wetness(const voxl_sky_weather *w);
// is precipitation falling right now?
int voxl_sky_weather_is_precip(const voxl_sky_weather *w);
const char *voxl_sky_weather_name(voxl_sky_weather_kind k);
#endif
