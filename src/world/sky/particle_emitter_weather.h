#ifndef WORLD_SKY_PARTICLE_EMITTER_WEATHER_H
#define WORLD_SKY_PARTICLE_EMITTER_WEATHER_H

// glue that drives the rain + snow fields from a weather state. picks which
// precip system is active based on the current weather kind and feeds it the
// ramped intensity. owns both fields so callers only touch one thing.

#include "../../math/vec3.h"
#include "../../math/vec4.h"
#include "weather_state.h"
#include "rain.h"
#include "snow.h"

typedef enum {
    VOXL_SKY_EMIT_NONE = 0,
    VOXL_SKY_EMIT_RAIN,
    VOXL_SKY_EMIT_SNOW
} voxl_sky_emit_mode;

typedef struct {
    voxl_sky_rain rain;
    voxl_sky_snow snow;
    voxl_sky_emit_mode mode;
} voxl_sky_weather_emitter;

void voxl_sky_emitter_init(voxl_sky_weather_emitter *e, unsigned seed,
                           float box, float top);

// step the emitter from a weather state. center is the player position.
void voxl_sky_emitter_update(voxl_sky_weather_emitter *e,
                             const voxl_sky_weather *w,
                             float dt, vec3 center);

// how many particles are currently live (rain or snow).
int voxl_sky_emitter_live_count(const voxl_sky_weather_emitter *e);

#endif
