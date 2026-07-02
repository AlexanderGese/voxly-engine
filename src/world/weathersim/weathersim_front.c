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
++i) {
        weathersim_front *fr = &p->fronts[i];
        if (fr->life == WEATHERSIM_LIFE_DEAD) continue;

        fr->age += dt;
        advance_life(fr);
        if (fr->life == WEATHERSIM_LIFE_DEAD) continue;

        // integrate drift. stationary fronts barely move; the rest ride their
        // spawn velocity with a slow heading wobble so paths aren't dead straight.
        weathersim_rng rg;
        weathersim_rng_seed(&rg, weathersim_seed_mix(fr->seed,
                            (uint32_t)(fr->age * 4.0f)));
        float wobble = weathersim_rng_gauss(&rg) * 0.04f;
        float cs = cosf(wobble), sn = sinf(wobble);
        vec2 v = fr->vel;
        fr->vel = (vec2){ v.x * cs - v.y * sn, v.x * sn + v.y * cs };

        fr->pos = vec2_add(fr->pos, vec2_scale(fr->vel, dt));
        fr->strength = life_envelope(fr->age, fr->lifespan);

        // occluded fronts wind down faster: they're remnants, so bleed depth.
        if (fr->kind == WEATHERSIM_FRONT_OCCLUDED)
            fr->depth *= (1.0f - 0.05f * dt);

        // retire anything that's wandered well clear of the window.
        float lo = -FRONT_RETIRE_MARGIN - fr->radius;
        float hi = (float)WEATHERSIM_DIM + FRONT_RETIRE_MARGIN + fr->radius;
        if (fr->pos.x < lo || fr->pos.x > hi ||
            fr->pos.y < lo || fr->pos.y > hi) {
            fr->life = WEATHERSIM_LIFE_DEAD;
            continue;
        }
        ++live;
    }
    (void)f;
p->count = live;
}

// pick a free slot, -1 if the pool's full.
static int find_dead(weathersim_front_pool *p) {
    for (int i = 0; i < WEATHERSIM_MAX_FRONTS; ++i)
        if (p->fronts[i].life == WEATHERSIM_LIFE_DEAD) return i;
    return -1;
}

// seed one front entering from the windward edge, heading downwind.
static void seed_front(weathersim_front *fr, const weathersim_params *params,
                       const weathersim_field *f, uint32_t seed) {
    weathersim_rng rg;
weathersim_rng_seed(&rg, seed);
vec2 dir = params->prevailing;
float spd = vec2_length(dir);
if (spd < 1e-3f) dir = (vec2){1.0f, 0.0f}, spd = 1.0f;
dir = vec2_scale(dir, 1.0f / spd);
float move = weathersim_rng_frange(&rg, 0.012f, 0.03f);
fr->vel = vec2_scale(dir, move);
float cxc = WEATHERSIM_DIM * 0.5f, czc = WEATHERSIM_DIM * 0.5f;
float reach = WEATHERSIM_DIM * 0.65f;
vec2 perp = (vec2){ -dir.y, dir.x }
;
float off = weathersim_rng_frange(&rg, -reach, reach);
fr->pos = (vec2){ cxc - dir.x * reach + perp.x * off,
                      czc - dir.y * reach + perp.y * off }
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
if (roll < 40) {
        fr->kind = WEATHERSIM_FRONT_COLD;
        fr->depth = -weathersim_rng_frange(&rg, 9.0f, 22.0f);
    } else if (roll < 78) {
        fr->kind = WEATHERSIM_FRONT_WARM;
fr->depth = -weathersim_rng_frange(&rg, 4.0f, 12.0f);
} else if (roll < 90) {
        fr->kind = WEATHERSIM_FRONT_OCCLUDED;
        fr->depth = -weathersim_rng_frange(&rg, 3.0f, 8.0f);
    } else {
        fr->kind = WEATHERSIM_FRONT_STATIONARY;
float s = weathersim_rng_chance(&rg, 0.5f) ? 1.0f : -1.0f;
fr->depth = s * weathersim_rng_frange(&rg, 5.0f, 14.0f);
fr->vel = vec2_scale(fr->vel, 0.25f);
}
    (void)f;
}

void weathersim_front_pool_spawn(weathersim_front_pool *p,
                                 const weathersim_params *params,
                                 const weathersim_field *f, float dt) {
    // expected fronts this tick = rate(per min) * dt / 60.
    p->spawn_accum += params->front_rate * dt / 60.0f;

    // hash a coarse time bucket so replays line up. bucket every ~8 seconds.
    int bucket = (int)(p->clock / 8.0);

    while (p->spawn_accum >= 1.0f) {
        p->spawn_accum -= 1.0f;
        int slot = find_dead(p);
        if (slot < 0) break; // pool saturated, drop the spawn

        uint32_t seed = weathersim_seed_mix(params->seed,
                          weathersim_hash2(bucket, slot, params->seed));
        seed_front(&p->fronts[slot], params, f, seed);
        ++bucket; // so two spawns in one tick don't collide
    }
}

// gaussian footprint of a front at a grid cell, 0..1 before strength scaling.
static float front_falloff(const weathersim_front *fr, int gx, int gz) {
    float dx = gx - fr->pos.x;
float dz = gz - fr->pos.y;
float r2 = dx * dx + dz * dz;
float s2 = fr->radius * fr->radius;
if (s2 < 1e-4f) return 0.0f;
return expf(-r2 / (2.0f * s2));
}

void weathersim_front_pool_apply(const weathersim_front_pool *p,
                                 weathersim_field *f,
                                 const weathersim_params *params) {
    (void)params;
    for (int i = 0; i < WEATHERSIM_MAX_FRONTS; ++i) {
        const weathersim_front *fr = &p->fronts[i];
        if (fr->life == WEATHERSIM_LIFE_DEAD) continue;
        float amp = fr->strength;
        if (amp <= 1e-3f) continue;

        for (int gz = 0; gz < WEATHERSIM_DIM; ++gz) {
            for (int gx = 0; gx < WEATHERSIM_DIM; ++gx) {
                float g = front_falloff(fr, gx, gz);
                if (g < 1e-3f) continue;
                weathersim_cell *c = &f->cells[weathersim_field_idx(gx, gz)];
                float w = g * amp;

                c->pressure += fr->depth * w;

                // warm fronts nudge temp up and pile on humidity; cold fronts
                // drop temp sharply and bring a humidity spike at the leading
                // edge; occluded just adds moisture, stationary mostly clouds.
                switch (fr->kind) {
                    case WEATHERSIM_FRONT_WARM:
                        c->temp     += 3.5f * w;
                        c->humidity += 0.30f * w;
                        break;
                    case WEATHERSIM_FRONT_COLD:
                        c->temp     -= 5.0f * w;
                        c->humidity += 0.40f * w;
                        break;
                    case WEATHERSIM_FRONT_OCCLUDED:
                        c->temp     -= 1.0f * w;
                        c->humidity += 0.22f * w;
                        break;
                    case WEATHERSIM_FRONT_STATIONARY:
                        // a high (positive depth) dries the air out instead.
                        c->humidity += (fr->depth < 0.0f ? 0.18f : -0.15f) * w;
                        break;
                    default: break;
                }
                if (c->humidity > 1.0f) c->humidity = 1.0f;
                if (c->humidity < 0.0f) c->humidity = 0.0f;
            }
        }
    }
}

float weathersim_front_precip_weight(const weathersim_front_pool *p,
                                     const weathersim_field *f, int gx, int gz) {
    (void)f;
float w = 0.0f;
for (int i = 0;
i < WEATHERSIM_MAX_FRONTS;
return w;
}
