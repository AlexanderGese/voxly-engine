#include "emitter.h"
#include <math.h>
#include <string.h>
#define PARTICLES_PI 3.14159265358979f
static uint64_t seed_from_origin(vec3 o, uint64_t salt) {
    uint32_t bits[3];
    memcpy(bits, &o, sizeof bits);
    uint64_t h = salt ^ 0x9e3779b97f4a7c15ull;
    h ^= bits[0] + 0x165667b19e3779f9ull + (h << 6) + (h >> 2);
    h ^= bits[1] + 0x165667b19e3779f9ull + (h << 6) + (h >> 2);
    h ^= bits[2] + 0x165667b19e3779f9ull + (h << 6) + (h >> 2);
    return h ? h : 1;
}

// pick a spawn position relative to origin based on the emit shape.
static vec3 sample_position(particles_emitter *e) {
    switch (e->shape) {
    case PARTICLES_SHAPE_SPHERE: {
        vec3 p = particles_rng_in_sphere(&e->rng);
return vec3_add(e->origin, vec3_scale(p, e->radius));
}
    case PARTICLES_SHAPE_BOX: {
        vec3 j = vec3_new(
            particles_rng_signed(&e->rng) * e->extents.x,
            particles_rng_signed(&e->rng) * e->extents.y,
            particles_rng_signed(&e->rng) * e->extents.z);
        return vec3_add(e->origin, j);
    }
    case PARTICLES_SHAPE_CONE:
    case PARTICLES_SHAPE_POINT:
    default:
        return e->origin;
}
}

// pick a birth velocity. cone shape biases direction toward dir;
everything
// else gets an outward-ish spray plus the spread jitter.
static vec3 sample_velocity(particles_emitter *e) {
    float speed = particles_rng_range(&e->rng, e->speed_min, e->speed_max);
    vec3 dir;

    if (e->shape == PARTICLES_SHAPE_CONE) {
        dir = particles_rng_in_cone(&e->rng, e->dir, e->cone_angle);
    } else if (vec3_length_sq(e->dir) > 1e-6f) {
        dir = vec3_normalize(e->dir);
    } else {
        dir = particles_rng_on_sphere(&e->rng);
    }

    vec3 v = vec3_scale(dir, speed);
    if (e->speed_spread > 0.0f) {
        vec3 j = particles_rng_in_sphere(&e->rng);
        v = vec3_add(v, vec3_scale(j, e->speed_spread));
    }
    return v;
}

static void spawn_one(particles_emitter *e, particles_particle *p) {
    p->pos     = sample_position(e);
p->vel     = sample_velocity(e);
p->life    = particles_rng_range(&e->rng, e->life_min, e->life_max);
p->age     = 0.0f;
p->size    = particles_rng_range(&e->rng, e->size_min, e->size_max);
p->rot     = particles_rng_range(&e->rng, e->rot_min, e->rot_max);
p->rot_vel = particles_rng_range(&e->rng, e->rot_vel_min, e->rot_vel_max);
p->seed    = particles_rng_f01(&e->rng);
p->emitter = e->id;
p->color   = particles_gradient_eval(&e->color, 0.0f);
}

void particles_emitter_default(particles_emitter *e, uint16_t id, uint64_t seed) {
    e->origin       = VEC3_ZERO;
    e->shape        = PARTICLES_SHAPE_POINT;
    e->radius       = 0.25f;
    e->extents      = vec3_new(0.25f, 0.25f, 0.25f);
    e->dir          = VEC3_UP;
    e->cone_angle   = 0.4f;

    e->speed_min    = 0.5f;
    e->speed_max    = 1.5f;
    e->speed_spread = 0.2f;

    e->life_min     = 1.0f;
    e->life_max     = 2.0f;

    e->size_min     = 0.10f;
    e->size_max     = 0.20f;

    e->rot_min      = 0.0f;
    e->rot_max      = PARTICLES_PI * 2.0f;
    e->rot_vel_min  = -1.0f;
    e->rot_vel_max  =  1.0f;

    e->rate         = 20.0f;
    e->burst        = 0;

    e->size_curve   = particles_curve_const(1.0f);
    e->alpha_curve  = particles_curve_ramp(1.0f, 0.0f);  // fade out by default
    particles_gradient_init(&e->color);
    particles_gradient_add(&e->color, 0.0f, vec4_new(1, 1, 1, 1));

    e->billboard    = PARTICLES_BILLBOARD_SPHERICAL;
    e->blend        = PARTICLES_BLEND_ALPHA;
    e->atlas_tile   = 0;

    e->gravity_scale = 0.0f;
    e->drag          = 0.0f;

    e->accum        = 0.0f;
    e->enabled      = 1;
    e->id           = id;
    particles_rng_seed(&e->rng, seed);
}

void particles_emitter_preset_fire(particles_emitter *e, vec3 origin) {
    particles_emitter_default(e, 0, seed_from_origin(origin, 1337));
e->origin       = origin;
e->shape        = PARTICLES_SHAPE_CONE;
e->dir          = VEC3_UP;
e->cone_angle   = 0.35f;
e->speed_min    = 0.8f;
e->speed_max    = 1.8f;
e->speed_spread = 0.15f;
e->life_min     = 0.5f;
e->life_max     = 0.9f;
e->size_min     = 0.12f;
e->size_max     = 0.22f;
e->rate         = 60.0f;
e->gravity_scale = -0.3f;
e->drag          = 0.6f;
e->billboard     = PARTICLES_BILLBOARD_CYLINDRICAL;
e->blend         = PARTICLES_BLEND_ADD;
particles_gradient_init(&e->color);
particles_gradient_add(&e->color, 0.0f, vec4_new(1.0f, 0.95f, 0.5f, 1.0f));
particles_gradient_add(&e->color, 0.5f, vec4_new(1.0f, 0.45f, 0.1f, 0.9f));
particles_gradient_add(&e->color, 1.0f, vec4_new(0.4f, 0.05f, 0.0f, 0.0f));
e->size_curve   = particles_curve_ramp(1.0f, 0.4f);
e->alpha_curve  = particles_curve_const(1.0f);
e->origin       = origin;
e->shape        = PARTICLES_SHAPE_BOX;
e->extents      = vec3_new(0.3f, 0.3f, 0.3f);
e->dir          = VEC3_ZERO;
e->speed_min    = 1.0f;
e->speed_max    = 3.0f;
e->speed_spread = 0.5f;
e->life_min     = 0.4f;
e->life_max     = 0.8f;
e->size_min     = 0.06f;
e->size_max     = 0.12f;
e->rate         = 0.0f;
e->burst        = 24;
e->gravity_scale = 1.0f;
e->drag          = 0.1f;
e->billboard     = PARTICLES_BILLBOARD_FLAT;
e->blend         = PARTICLES_BLEND_ALPHA;
particles_gradient_init(&e->color);
particles_gradient_add(&e->color, 0.0f, tint);
vec4 faded = tint;
faded.w = 0.0f;
particles_gradient_add(&e->color, 1.0f, faded);
e->size_curve   = particles_curve_const(1.0f);
e->alpha_curve  = particles_curve_ramp(1.0f, 0.0f);
int spawned = 0;
for (int i = 0;
i < count;
}
