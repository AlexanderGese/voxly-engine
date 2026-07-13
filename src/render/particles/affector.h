#ifndef RENDER_PARTICLES_AFFECTOR_H
#define RENDER_PARTICLES_AFFECTOR_H

// force fields the simulator folds in on top of plain gravity/drag. these are
// global to a system (not per emitter) — a gust of wind hits everything. kept
// as a small fixed array so the sim inner loop stays branch-light.
//
// these don't move particles directly; they accumulate an acceleration that
// the integrator adds before the position step. see particles_affector_accel.

#include "../../math/vec3.h"
#include "particle_types.h"

#define PARTICLES_MAX_AFFECTORS 8

typedef enum {
    PARTICLES_AFFECTOR_WIND = 0,   // constant directional push
    PARTICLES_AFFECTOR_POINT,      // attract/repel toward a point
    PARTICLES_AFFECTOR_VORTEX,     // swirl around an axis through a point
    PARTICLES_AFFECTOR_TURBULENCE  // pseudo-random per-particle jitter
} particles_affector_kind;

typedef struct {
    particles_affector_kind kind;
    int   enabled;

    vec3  vector;     // wind dir*strength, or vortex axis
    vec3  point;      // center for point/vortex
    float strength;   // accel magnitude
    float radius;     // falloff radius (0 = infinite). beyond it, no effect.
    float falloff;    // 1 = linear to zero at radius, 2 = quadratic
} particles_affector;

typedef struct {
    particles_affector items[PARTICLES_MAX_AFFECTORS];
    int count;
    float time;       // advanced by the sim, drives turbulence phase
} particles_affector_set;

void particles_affectors_init(particles_affector_set *set);
int  particles_affectors_add(particles_affector_set *set,
                             const particles_affector *a);

// ready-made constructors for the common ones.
particles_affector particles_affector_wind(vec3 dir, float strength);
particles_affector particles_affector_point(vec3 center, float strength,
                                            float radius);
particles_affector particles_affector_vortex(vec3 center, vec3 axis,
                                             float strength, float radius);

// total acceleration on one particle from all enabled affectors. the sim adds
// this to gravity each step. `set->time` should be advanced once per frame.
vec3 particles_affector_accel(const particles_affector_set *set,
                              const particles_particle *p);

#endif
