#ifndef ENTITY_MOBS_MOB_RNG_H
#define ENTITY_MOBS_MOB_RNG_H

#include <stdint.h>

// tiny self contained prng so the mob modules don't depend on the engine
// rng layout. xorshift32, good enough for picking wander angles.

typedef struct {
    uint32_t s;
} voxl_mob_rng;

// seed it. a 0 seed gets nudged so we never get stuck at all zeros.
void  voxl_mob_rng_seed(voxl_mob_rng *r, uint32_t seed);

uint32_t voxl_mob_rng_u32(voxl_mob_rng *r);

// float in [0,1).
float voxl_mob_rng_f01(voxl_mob_rng *r);

// float in [lo,hi).
float voxl_mob_rng_frange(voxl_mob_rng *r, float lo, float hi);

// int in [lo,hi] inclusive.
int   voxl_mob_rng_range(voxl_mob_rng *r, int lo, int hi);

// 1 with probability p, else 0.
int   voxl_mob_rng_chance(voxl_mob_rng *r, float p);

#endif
