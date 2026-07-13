#ifndef RENDER_PARTICLES_PARTICLE_RNG_H
#define RENDER_PARTICLES_PARTICLE_RNG_H
// spawn-side randomness. i could've reused math/rng directly but the spawn
// path wants a pile of small helpers (cone dirs, jittered points, unit
// vectors) and i didn't want to bloat the shared rng header with them.
// thin wrapper over the same xorshift64* so seeding behaves identically.
#include "../../math/vec3.h"
#include <stdint.h>
typedef struct { uint64_t s; } particles_rng;
void  particles_rng_seed(particles_rng *r, uint64_t seed);
float particles_rng_f01(particles_rng *r);
// [0,1)
float particles_rng_range(particles_rng *r, float lo, float hi);
// signed [-1,1)
float particles_rng_signed(particles_rng *r);
// a random point inside the unit sphere (rejection, but cheap on average).
vec3  particles_rng_in_sphere(particles_rng *r);
// a random direction on the unit sphere. uniform.
vec3  particles_rng_on_sphere(particles_rng *r);
// a random direction inside a cone around `axis`, half-angle in radians.
// used by directional emitters (fountains, jets).
vec3  particles_rng_in_cone(particles_rng *r, vec3 axis, float half_angle);
#endif
