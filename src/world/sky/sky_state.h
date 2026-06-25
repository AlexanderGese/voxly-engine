#ifndef WORLD_SKY_STATE_H
#define WORLD_SKY_STATE_H

// the top-level sky module. owns the clock, weather, clouds, stars, lightning
// and precip, and bakes a per-frame snapshot of everything the renderer needs
// (colors, light dirs, fog, flash). update once a frame, read the snapshot.

#include "../../math/vec3.h"
#include "../../math/vec4.h"

#include "weather_state.h"
#include "ambient_light.h"
#include "fog_density.h"
#include "celestial.h"
#include "cloud.h"
#include "starfield.h"
#include "lightning.h"
#include "particle_emitter_weather.h"
#include "moon_phase.h"
#include "season.h"

typedef struct {
    vec3  fog_color;
    vec3  zenith;
    vec3  horizon;
    voxl_sky_lighting light;
    voxl_sky_fog fog;
    voxl_sky_body sun;
    voxl_sky_body moon;
    voxl_moon_phase moon_phase;
    voxl_sky_season season;
    float star_visibility;
    float lightning_flash;
    float wetness;
} voxl_sky_snapshot;

typedef struct {
    float hours;          // current in-game hour 0..24
    float rate;           // in-game hours per real second
    long  day;            // whole days elapsed
    float view_dist;      // far plane in blocks (for fog)
    int   days_per_season;

    voxl_sky_weather weather;
    voxl_sky_clouds  clouds;
    voxl_sky_starfield stars;
    voxl_sky_lightning lightning;
    voxl_sky_weather_emitter emitter;

    float time_s;         // wall-clock seconds accumulator (twinkle/sway)
    voxl_sky_snapshot snap;
} voxl_sky_state;

// set everything up. seed feeds all the sub-rngs; start_hour 0..24.
void voxl_sky_state_init(voxl_sky_state *s, unsigned seed, float start_hour);

// advance clock + all subsystems, then rebake the snapshot. center is the
// player position (for precip + lightning placement).
void voxl_sky_state_update(voxl_sky_state *s, float dt, vec3 center);

// convenience read of the current snapshot.
const voxl_sky_snapshot *voxl_sky_state_snapshot(const voxl_sky_state *s);

#endif
