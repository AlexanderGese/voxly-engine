#include "sky_state.h"
#include "sky_math.h"
#include "sky_gradient.h"
#include "sky_color.h"

void voxl_sky_state_init(voxl_sky_state *s, unsigned seed, float start_hour) {
    s->hours = voxl_sky_wrap24(start_hour);
    s->rate  = 1.0f / 30.0f;     // match daynight.c: 30 real sec per hour
    s->day   = 0;
    s->view_dist = 256.0f;
    s->days_per_season = 16;
    s->time_s = 0.0f;

    voxl_sky_weather_init(&s->weather, seed);
    voxl_sky_clouds_init(&s->clouds, seed ^ 0x1111u);
    voxl_sky_starfield_init(&s->stars, VOXL_SKY_STAR_MAX, seed ^ 0x2222u);
    voxl_sky_lightning_init(&s->lightning, seed ^ 0x3333u);
    voxl_sky_emitter_init(&s->emitter, seed ^ 0x4444u, s->view_dist * 0.25f, 32.0f);

    // bake an initial snapshot so readers never see garbage.
    voxl_sky_state_update(s, 0.0f, (vec3){0.0f, 0.0f, 0.0f});
}

static void advance_clock(voxl_sky_state *s, float dt) {
    s->time_s += dt;
    s->hours += dt * s->rate;
    while (s->hours >= 24.0f) {
        s->hours -= 24.0f;
        s->day++;
    }
}

void voxl_sky_state_update(voxl_sky_state *s, float dt, vec3 center) {
    if (dt < 0.0f) dt = 0.0f;
    advance_clock(s, dt);

    // tick subsystems.
    voxl_sky_weather_update(&s->weather, dt);
    float wet = voxl_sky_weather_wetness(&s->weather);
    voxl_sky_clouds_update(&s->clouds, dt, wet);

    float storm = (s->weather.kind == VOXL_SKY_STORM) ? s->weather.intensity : 0.0f;
    voxl_sky_lightning_update(&s->lightning, dt, storm, center);
    voxl_sky_emitter_update(&s->emitter, &s->weather, dt, center);

    // bake the snapshot.
    voxl_sky_snapshot *o = &s->snap;
    float h = s->hours;

    voxl_sky_band band = voxl_sky_gradient_band(h);
    o->zenith  = voxl_sky_apply_overcast(band.zenith,  wet);
    o->horizon = voxl_sky_apply_overcast(band.horizon, wet);
    o->fog_color = voxl_sky_apply_overcast(voxl_sky_fog_color(h), wet);

    o->light = voxl_sky_lighting_at(h, wet);
    o->fog   = voxl_sky_fog_params(h, wet, s->view_dist);
    o->sun   = voxl_sky_sun(h);
    o->moon  = voxl_sky_moon(h);
    o->moon_phase = voxl_sky_moon_phase(s->day);
    o->season = voxl_sky_season_at(s->day, s->days_per_season);
    o->star_visibility = voxl_sky_star_visibility(h);
    o->lightning_flash = voxl_sky_lightning_flash(&s->lightning);
    o->wetness = wet;

    // a lightning flash briefly lifts ambient + fog color toward white.
    if (o->lightning_flash > 0.0f) {
        float f = o->lightning_flash * 0.6f;
        vec3 white = {1.0f, 1.0f, 1.0f};
        o->fog_color = voxl_sky_mix3(o->fog_color, white, f);
        o->light.ambient = voxl_sky_mix3(o->light.ambient, white, f);
    }
}

const voxl_sky_snapshot *voxl_sky_state_snapshot(const voxl_sky_state *s) {
    return &s->snap;
}
