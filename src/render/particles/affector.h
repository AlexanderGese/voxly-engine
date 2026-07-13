#ifndef RENDER_PARTICLES_AFFECTOR_H
#define RENDER_PARTICLES_AFFECTOR_H
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
#endif
