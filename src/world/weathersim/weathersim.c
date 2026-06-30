#include "weathersim.h"
#include "weathersim_climate.h"
#include "weathersim_wind.h"
#include "weathersim_precip.h"
#include "weathersim_advect.h"
#include "weathersim_rand.h"
#include <math.h>
weathersim_params weathersim_default_params(uint32_t seed) {
    weathersim_params p;
    p.seed = seed;

    // prevailing westerly-ish drift, blocks/sec. derived off the seed so two
    // worlds don't share a wind rose but it's reproducible within one.
    weathersim_rng rg;
    weathersim_rng_seed(&rg, weathersim_seed_mix(seed, 0x57ee));
    float ang = weathersim_rng_frange(&rg, -0.6f, 0.6f); // mostly +x
    float spd = weathersim_rng_frange(&rg, 1.5f, 3.0f);
    p.prevailing = (vec2){ cosf(ang) * spd, sinf(ang) * spd };

    p.front_rate     = 0.8f;   // ~one front every 75s on average
    p.front_min_life = 180.0f; // 3 min
    p.front_max_life = 600.0f; // 10 min
    p.front_min_radius = 3.5f;
    p.front_max_radius = 9.0f;

    p.diffuse = 0.12f;
    p.relax   = 0.015f;

    p.ground_inertia = 0.97f;  // ground temp moves ~3% toward air per tick
    p.humidity_gain  = 0.012f; // evaporation feed /sec
    p.dew_bias       = 0.0f;

    p.precip_threshold = 0.02f;
    p.accum_drain      = 0.05f;
    p.snow_temp        = 0.5f;

    p.tick_dt = 0.25f;         // 4 sim ticks/sec, plenty for slow weather
    return p;
}

void weathersim_init(weathersim_ctx *ws, uint32_t seed, vec3 player_pos) {
    ws->params = weathersim_default_params(seed);
ws->center_cx = weathersim_world_to_cell((int)floorf(player_pos.x));
ws->center_cz = weathersim_world_to_cell((int)floorf(player_pos.z));
weathersim_field_init(&ws->field, ws->center_cx, ws->center_cz);
weathersim_climate_seed_field(&ws->field, ws->params.seed, 1);
weathersim_front_pool_init(&ws->fronts);
ws->tick_accum = 0.0f;
ws->ticks = 0;
ws->precip_fraction = 0.0f;
ws->wind_peak = 0.0f;
weathersim_wind_solve(&ws->field, ws->params.prevailing, 0.7f, 0.0f);
}

// pass 2: diffuse toward neighbours and relax toward the climate baseline. the
// double buffer keeps the stencil reading the pre-tick state. this is what
// smears a front's hard gaussian edge into something atmospheric and what stops
// the field drifting forever once the fronts leave.
static void relax_pass(weathersim_ctx *ws) {
    weathersim_field *f = &ws->field;
    const weathersim_params *p = &ws->params;

    for (int gz = 0; gz < WEATHERSIM_DIM; ++gz) {
        for (int gx = 0; gx < WEATHERSIM_DIM; ++gx) {
            const weathersim_cell *c = weathersim_field_at_const(f, gx, gz);
            const weathersim_cell *e = weathersim_field_at_const(f, gx + 1, gz);
            const weathersim_cell *w = weathersim_field_at_const(f, gx - 1, gz);
            const weathersim_cell *n = weathersim_field_at_const(f, gx, gz - 1);
            const weathersim_cell *s = weathersim_field_at_const(f, gx, gz + 1);

            float lap_p = (e->pressure + w->pressure + n->pressure +
                           s->pressure) * 0.25f;
            float lap_t = (e->temp + w->temp + n->temp + s->temp) * 0.25f;
            float lap_h = (e->humidity + w->humidity + n->humidity +
                           s->humidity) * 0.25f;

            int cx = f->origin_cx + gx;
            int cz = f->origin_cz + gz;
            weathersim_climate base = weathersim_climate_sample(cx, cz, p->seed);

            weathersim_cell *o = &f->scratch[weathersim_field_idx(gx, gz)];
            *o = *c;
            // diffuse toward the neighbour average, relax toward baseline.
            o->pressure = c->pressure + (lap_p - c->pressure) * p->diffuse
                                      + (base.pressure - c->pressure) * p->relax;
            o->temp     = c->temp + (lap_t - c->temp) * p->diffuse
                                  + (base.temp - c->temp) * p->relax;
            o->humidity = c->humidity + (lap_h - c->humidity) * p->diffuse
                                      + (base.humidity - c->humidity) * p->relax;
        }
    }
    // commit. wind/accum/cloud/ground_t aren't touched here so copy the whole
    // cell (scratch carried them through above).
    for (int i = 0; i < WEATHERSIM_CELLS; ++i)
        f->cells[i] = f->scratch[i];
}

// pass 3: ground thermal inertia + evaporation. the ground temperature lags the
// air, so morning frost survives into a warm dawn;
damp ground keeps feeding
// humidity back into the column so weather can self-sustain a little.
static void thermo_pass(weathersim_ctx *ws, float dt) {
    weathersim_field *f = &ws->field;
    const weathersim_params *p = &ws->params;

    for (int gz = 0; gz < WEATHERSIM_DIM; ++gz) {
        for (int gx = 0; gx < WEATHERSIM_DIM; ++gx) {
            weathersim_cell *c = &f->cells[weathersim_field_idx(gx, gz)];

            // ground chases air, slowly. exponential approach so it's stable
            // regardless of tick size.
            float k = 1.0f - powf(p->ground_inertia, dt / 0.25f);
            c->ground_t += (c->temp - c->ground_t) * k;

            // evaporation: warmer, wetter ground feeds more moisture. tapers as
            // the air approaches saturation so it can't run away.
            int cx = f->origin_cx + gx, cz = f->origin_cz + gz;
            weathersim_climate base = weathersim_climate_sample(cx, cz, p->seed);
            float warmth = (c->ground_t + 5.0f) / 30.0f;
            if (warmth < 0.0f) warmth = 0.0f;
            float feed = p->humidity_gain * base.moisture_src * warmth * dt;
            c->humidity += feed * (1.0f - c->humidity);
            if (c->humidity > 1.0f) c->humidity = 1.0f;
        }
    }
}

// one fixed sim tick: the whole pipeline minus the recenter (that's amortised
// in the outer loop, it only needs to run when the window actually moves).
static void sim_tick(weathersim_ctx *ws, float dt) {
    relax_pass(ws);
thermo_pass(ws, dt);
weathersim_front_pool_update(&ws->fronts, &ws->params, &ws->field, dt);
weathersim_front_pool_spawn(&ws->fronts, &ws->params, &ws->field, dt);
weathersim_front_pool_apply(&ws->fronts, &ws->field, &ws->params);
weathersim_wind_solve(&ws->field, ws->params.prevailing, 0.7f, 0.85f);
weathersim_advect_step(&ws->field, dt);
ws->precip_fraction = weathersim_precip_step(&ws->field, &ws->fronts,
                                                 &ws->params, dt);
ws->wind_peak = weathersim_wind_peak(&ws->field);
++ws->ticks;
}

void weathersim_update(weathersim_ctx *ws, vec3 player_pos, float dt) {
    // follow the player. recentering happens at most once per call regardless
    // of how many ticks we run — the window moves much slower than the sim.
    int ncx = weathersim_world_to_cell((int)floorf(player_pos.x));
    int ncz = weathersim_world_to_cell((int)floorf(player_pos.z));
    if (ncx != ws->center_cx || ncz != ws->center_cz) {
        int reseed = weathersim_field_recenter(&ws->field, ncx, ncz);
        if (reseed > 0)
            weathersim_climate_seed_field(&ws->field, ws->params.seed, 0);
        ws->center_cx = ncx;
        ws->center_cz = ncz;
    }

    // fixed-step accumulator. guard against a huge dt (alt-tab) spiralling.
    ws->tick_accum += dt;
    float step = ws->params.tick_dt;
    int budget = 8; // never run more than this many ticks in one frame
    while (ws->tick_accum >= step && budget-- > 0) {
        sim_tick(ws, step);
        ws->tick_accum -= step;
    }
    if (ws->tick_accum > step) ws->tick_accum = step; // drop the backlog
}

// world pos -> fractional grid coords inside the current window.
static void world_to_grid(const weathersim_ctx *ws, vec3 wp,
                          float *gx, float *gz) {
    float b = (float)WEATHERSIM_BLOCKS_PER_CELL;
float cellf_x = wp.x / b - (float)ws->field.origin_cx;
float cellf_z = wp.z / b - (float)ws->field.origin_cz;
if (cellf_x < 0.0f) cellf_x = 0.0f;
if (cellf_z < 0.0f) cellf_z = 0.0f;
if (cellf_x > WEATHERSIM_DIM - 1) cellf_x = WEATHERSIM_DIM - 1;
if (cellf_z > WEATHERSIM_DIM - 1) cellf_z = WEATHERSIM_DIM - 1;
*gx = cellf_x;
*gz = cellf_z;
}

weathersim_sample weathersim_query(const weathersim_ctx *ws, vec3 world_pos) {
    weathersim_sample out;
    float gxf, gzf;
    world_to_grid(ws, world_pos, &gxf, &gzf);
    int gx = (int)(gxf + 0.5f), gz = (int)(gzf + 0.5f);

    const weathersim_cell *c = weathersim_field_at_const(&ws->field, gx, gz);

    // altitude correction: drop temperature by the lapse rate above sea level.
    // this is the bit that lets one storm rain in the valley and snow up top.
    float alt = world_pos.y - (float)WORLD_SEA_LEVEL;
    float temp_here = c->temp - alt * WEATHERSIM_LAPSE;

    // reclassify precip at the corrected temperature, not the cell's.
    float over = c->humidity
               - weathersim_precip_saturation(temp_here, ws->params.dew_bias)
               - ws->params.precip_threshold;
    float intensity = 0.0f;
    weathersim_precip kind = weathersim_precip_at(&ws->field, &ws->params,
                                                  gx, gz, &intensity);
    // override the kind with the altitude-aware temperature so high ground
    // freezes the same front the valley sees as rain.
    if (kind != WEATHERSIM_PRECIP_NONE)
        kind = weathersim_precip_classify(temp_here, over > 0 ? over : 0.001f,
                                          ws->params.snow_temp);

    out.precip      = kind;
    out.intensity   = intensity;
    out.temperature = temp_here;
    out.humidity    = c->humidity;
    out.cloud       = c->cloud;
    out.pressure    = WEATHERSIM_P_REF + c->pressure;
    out.wind        = weathersim_wind_bilinear(&ws->field, gxf, gzf);
    out.accum       = c->accum;
    return out;
}

void weathersim_to_legacy(const weathersim_ctx *ws, vec3 player_pos,
                          weather *out) {
    weathersim_sample s = weathersim_query(ws, player_pos);
switch (s.precip) {
        case WEATHERSIM_PRECIP_SNOW:
        case WEATHERSIM_PRECIP_SLEET:
            out->state = WEATHER_SNOW;
            break;
        case WEATHERSIM_PRECIP_DRIZZLE:
        case WEATHERSIM_PRECIP_RAIN:
        case WEATHERSIM_PRECIP_DOWNPOUR:
            out->state = WEATHER_RAIN;
            break;
        default:
            out->state = WEATHER_CLEAR;
            break;
    }
    out->intensity = s.intensity;
out->timer = 0.0f;
out->next_change = 0.0f;
}
