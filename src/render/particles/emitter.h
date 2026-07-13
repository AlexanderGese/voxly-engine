#ifndef RENDER_PARTICLES_EMITTER_H
#define RENDER_PARTICLES_EMITTER_H
// an emitter is a description of how to spawn particles plus a little bit of
// running state (accumulator, burst timers). it does not own a pool — the
// system hands it one. this keeps emitters cheap to copy around and lets a
// single pool be shared by many emitters (fire + smoke off the same torch).
#include "../../math/vec3.h"
#include "../../math/vec4.h"
#include "particle_types.h"
#include "particle_rng.h"
#include "curves.h"
#include "particle_pool.h"
typedef enum {
    PARTICLES_SHAPE_POINT = 0,   // all from one spot
    PARTICLES_SHAPE_SPHERE,      // inside a sphere of `radius`
    PARTICLES_SHAPE_BOX,         // inside a box of half-extents `extents`
    PARTICLES_SHAPE_CONE         // directional, around `dir`, `cone_angle`
} particles_emit_shape;
typedef struct {
    // where / what shape
    vec3  origin;
    particles_emit_shape shape;
    float radius;          // sphere
    vec3  extents;         // box half-extents
    vec3  dir;             // cone axis (also base velocity dir)
    float cone_angle;      // radians, cone half-angle

    // initial velocity. speed is sampled in [speed_min, speed_max] along the
    // spawn direction, plus a random spread component.
    float speed_min, speed_max;
    float speed_spread;    // extra omnidirectional jitter

    // lifetime sampled in [life_min, life_max]
    float life_min, life_max;

    // size sampled in [size_min, size_max] at birth, then scaled by curve
    float size_min, size_max;

    // roll
    float rot_min, rot_max;       // initial roll
    float rot_vel_min, rot_vel_max;

    // emission rate. continuous particles/sec, plus one-shot bursts.
    float rate;
    int   burst;           // particles to emit on the next explicit burst

    // over-lifetime shaping
    particles_curve    size_curve;   // multiplies birth size
    particles_curve    alpha_curve;  // multiplies gradient alpha
    particles_gradient color;        // rgba over life

    // rendering hints carried to the billboard stage
    particles_billboard_kind billboard;
    particles_blend_mode     blend;
    int   atlas_tile;      // tile id into the particle atlas

    // physics knobs the simulator reads (gravity scale, drag)
    float gravity_scale;
    float drag;

    // running state ---------------------------------------------------------
    float  accum;          // fractional particles carried between frames
    int    enabled;
    uint16_t id;           // stamped into spawned particles
    particles_rng rng;
} particles_emitter;
void particles_emitter_default(particles_emitter *e, uint16_t id, uint64_t seed);
void particles_emitter_preset_fire(particles_emitter *e, vec3 origin);
void particles_emitter_preset_smoke(particles_emitter *e, vec3 origin);
void particles_emitter_preset_block_break(particles_emitter *e, vec3 origin, vec4 tint);
int particles_emitter_emit(particles_emitter *e, particles_pool *pool, float dt);
int particles_emitter_burst(particles_emitter *e, particles_pool *pool, int count);
#endif
