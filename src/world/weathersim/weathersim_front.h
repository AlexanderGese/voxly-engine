#ifndef WORLD_WEATHERSIM_FRONT_H
#define WORLD_WEATHERSIM_FRONT_H

#include "weathersim_types.h"
#include "weathersim_field.h"

// the front system: a fixed pool of pressure systems that drift across the
// field, advance through a forming/mature/decaying lifecycle, and stamp their
// signature onto the grid every tick. this is where the "interesting" weather
// comes from — climate is the boring floor, fronts are the drama.
//
// spawning is poisson-ish: each tick we roll against an expected rate and, if
// it hits, seed a front at a windward edge so it sweeps across rather than
// popping into view in the middle. the seed is hashed off a coarse time bucket
// so a replayed world gets the same storms.

typedef struct {
    weathersim_front fronts[WEATHERSIM_MAX_FRONTS];
    int   count;             // live (non-dead) fronts, for the hud
    float spawn_accum;       // fractional fronts owed, carried between ticks
    double clock;            // seconds since sim start, drives the time bucket
} weathersim_front_pool;

void weathersim_front_pool_init(weathersim_front_pool *p);

// advance every live front: age it, walk its lifecycle state machine, integrate
// its drift, and retire it when it dies or sails off the (padded) window.
void weathersim_front_pool_update(weathersim_front_pool *p,
                                  const weathersim_params *params,
                                  const weathersim_field *f, float dt);

// roll for new fronts this tick and seed any that spawn.
void weathersim_front_pool_spawn(weathersim_front_pool *p,
                                 const weathersim_params *params,
                                 const weathersim_field *f, float dt);

// stamp every live front's pressure/temp/humidity signature into the field.
// additive over the climate-relaxed base; call after the relax pass.
void weathersim_front_pool_apply(const weathersim_front_pool *p,
                                 weathersim_field *f,
                                 const weathersim_params *params);

// how strongly the fronts are dumping precip at a grid cell, 0..1. used by the
// precip pass instead of re-deriving the gaussian footprints.
float weathersim_front_precip_weight(const weathersim_front_pool *p,
                                     const weathersim_field *f, int gx, int gz);

const char *weathersim_front_kind_name(weathersim_front_kind k);
const char *weathersim_front_life_name(weathersim_life l);

#endif
