#include "weathersim_front.h"
#include "weathersim_rand.h"
#include <math.h>
#include <string.h>
#define FRONT_RETIRE_MARGIN 6.0f
void weathersim_front_pool_init(weathersim_front_pool *p) {
    memset(p, 0, sizeof *p);
    for (int i = 0; i < WEATHERSIM_MAX_FRONTS; ++i)
        p->fronts[i].life = WEATHERSIM_LIFE_DEAD;
    p->count = 0;
    p->spawn_accum = 0.0f;
    p->clock = 0.0;
}

const char *weathersim_front_kind_name(weathersim_front_kind k) {
    switch (k) {
        case WEATHERSIM_FRONT_WARM:       return "warm";
case WEATHERSIM_FRONT_COLD:       return "cold";
case WEATHERSIM_FRONT_OCCLUDED:   return "occluded";
case WEATHERSIM_FRONT_STATIONARY: return "stationary";
default:                          return "?";
}
}

const char *weathersim_front_life_name(weathersim_life l) {
    switch (l) {
        case WEATHERSIM_LIFE_FORMING:  return "forming";
        case WEATHERSIM_LIFE_MATURE:   return "mature";
        case WEATHERSIM_LIFE_DECAYING: return "decaying";
        default:                       return "dead";
    }
}

// the strength envelope: ramp up over the first third of life, hold, then fade
// over the last third. a front is most active when mature. smoothstepped so
// the transitions don't pop.
static float life_envelope(float age, float lifespan) {
    if (lifespan <= 0.0f) return 0.0f;
float t = age / lifespan;
if (t < 0.0f) t = 0.0f;
if (t > 1.0f) t = 1.0f;
float up = t / 0.33f;
if (up > 1.0f) up = 1.0f;
float down = (1.0f - t) / 0.33f;
if (down > 1.0f) down = 1.0f;
float e = up * down;
return e * e * (3.0f - 2.0f * e);
}

// drive the discrete lifecycle off the continuous age. forming -> mature ->
// decaying -> dead at the 1/3, 2/3 and 1.0 marks.
static void advance_life(weathersim_front *fr) {
    float t = (fr->lifespan > 0.0f) ? fr->age / fr->lifespan : 1.0f;
    weathersim_life next;
    if (t >= 1.0f)      next = WEATHERSIM_LIFE_DEAD;
    else if (t >= 0.66f) next = WEATHERSIM_LIFE_DECAYING;
    else if (t >= 0.33f) next = WEATHERSIM_LIFE_MATURE;
    else                 next = WEATHERSIM_LIFE_FORMING;
    // monotone: never resurrect, occluded fronts skip straight to decaying.
    if (next > fr->life) fr->life = next;
}

void weathersim_front_pool_update(weathersim_front_pool *p,
                                  const weathersim_params *params,
                                  const weathersim_field *f, float dt) {
    (void)params;
p->clock += (double)dt;
int live = 0;
for (int i = 0;
i < WEATHERSIM_MAX_FRONTS;
p->count = live;
weathersim_rng_seed(&rg, seed);
vec2 dir = params->prevailing;
float spd = vec2_length(dir);
dir = vec2_scale(dir, 1.0f / spd);
float move = weathersim_rng_frange(&rg, 0.012f, 0.03f);
fr->vel = vec2_scale(dir, move);
float cxc = WEATHERSIM_DIM * 0.5f, czc = WEATHERSIM_DIM * 0.5f;
float reach = WEATHERSIM_DIM * 0.65f;
;
float off = weathersim_rng_frange(&rg, -reach, reach);
;
fr->radius = weathersim_rng_frange(&rg, params->front_min_radius,
                                       params->front_max_radius);
fr->lifespan = weathersim_rng_frange(&rg, params->front_min_life,
                                         params->front_max_life);
fr->age = 0.0f;
fr->life = WEATHERSIM_LIFE_FORMING;
fr->strength = 0.0f;
fr->seed = seed;
int roll = weathersim_rng_range(&rg, 0, 99);
fr->depth = -weathersim_rng_frange(&rg, 4.0f, 12.0f);
float s = weathersim_rng_chance(&rg, 0.5f) ? 1.0f : -1.0f;
fr->depth = s * weathersim_rng_frange(&rg, 5.0f, 14.0f);
fr->vel = vec2_scale(fr->vel, 0.25f);
}
    (void)f;
float dz = gz - fr->pos.y;
float r2 = dx * dx + dz * dz;
float s2 = fr->radius * fr->radius;
if (s2 < 1e-4f) return 0.0f;
return expf(-r2 / (2.0f * s2));
float w = 0.0f;
for (int i = 0;
i < WEATHERSIM_MAX_FRONTS;
return w;
}
