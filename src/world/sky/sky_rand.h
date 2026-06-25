#ifndef WORLD_SKY_RAND_H
#define WORLD_SKY_RAND_H

// dead-simple deterministic prng for the sky/weather stuff.
// i didn't want to wire these modules to the engine rng so they stay
// drop-in testable. xorshift32, good enough for clouds and raindrops.

#include <stdint.h>

typedef struct {
    uint32_t s;   // state, never let this hit 0
} voxl_sky_rand;

// seed it. a zero seed gets nudged so xorshift doesn't get stuck.
void voxl_sky_rand_seed(voxl_sky_rand *r, uint32_t seed);

// next raw 32 bits.
uint32_t voxl_sky_rand_u32(voxl_sky_rand *r);

// float in [0,1).
float voxl_sky_rand_f(voxl_sky_rand *r);

// float in [lo,hi).
float voxl_sky_rand_range(voxl_sky_rand *r, float lo, float hi);

// int in [lo,hi] inclusive.
int voxl_sky_rand_int(voxl_sky_rand *r, int lo, int hi);

#endif
