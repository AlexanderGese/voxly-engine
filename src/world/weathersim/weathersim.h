#ifndef WORLD_WEATHERSIM_H
#define WORLD_WEATHERSIM_H
#include <stddef.h>
#include "weathersim_types.h"
#include "weathersim_field.h"
#include "weathersim_front.h"
#include "../weather.h"
// public face of the weather simulator. one ctx owns the coarse field, the
// front pool and the params; you feed it the player position and a dt each
// frame and it advances a fixed-timestep simulation underneath, then you query
// it per-column for whatever the renderer / particles / audio need.
//
// the pipeline per sim tick, in order:
// 1. recenter   slide the field window to follow the player, seed new strip
// 2. relax      diffuse neighbours + pull cells back toward climate baseline
// 3. thermo     ground temp lags air, evaporation feeds humidity
// 4. fronts     update lifecycles, spawn new ones, stamp their signatures
// 5. wind       resolve the geostrophic wind field from the new pressure
// 6. advect     carry temp/moisture/cloud downwind along that field
// 7. precip     condense moisture, accumulate + drain, update clouds
//
// each numbered stage lives in its own translation unit; this file just owns
// the loop and the accumulator. nothing here touches a chunk.
typedef struct {
    weathersim_field      field;
    weathersim_front_pool fronts;
    weathersim_params     params;

    float  tick_accum;     // leftover real time owed to the fixed-step loop
    int    center_cx;      // current window center, cell space
    int    center_cz;
    int    ticks;          // total sim ticks run, for telemetry

    // last-tick stats, handy for a debug overlay or the engine log line.
    float  precip_fraction; // 0..1 of cells precipitating
    float  wind_peak;       // strongest gust, blocks/sec
} weathersim_ctx;
// set up a context around an initial player world position. allocates nothing
// beyond the ctx itself (the field is inline), seeds the field from climate.
void weathersim_init(weathersim_ctx *ws, uint32_t seed, vec3 player_pos);
// advance the simulation by real time `dt`, following the player. internally
// steps zero or more fixed sim ticks. cheap to call every frame.
void weathersim_update(weathersim_ctx *ws, vec3 player_pos, float dt);
// query the resolved weather at a world position (altitude matters: temp falls
// off with the lapse rate, so mountaintops get snow when the valley gets rain).
weathersim_sample weathersim_query(const weathersim_ctx *ws, vec3 world_pos);
// bridge to the existing engine enum in world/weather.h. fills a `weather`
// struct so render code that only knows clear/rain/snow keeps working, no
// changes needed on their side.
void weathersim_to_legacy(const weathersim_ctx *ws, vec3 player_pos,
                          weather *out);
#endif
