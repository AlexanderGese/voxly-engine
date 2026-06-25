#ifndef WORLD_SKY_PARTICLE_EMITTER_WEATHER_H
#define WORLD_SKY_PARTICLE_EMITTER_WEATHER_H
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
#endif
