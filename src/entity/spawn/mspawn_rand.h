#ifndef ENTITY_SPAWN_MSPAWN_RAND_H
#define ENTITY_SPAWN_MSPAWN_RAND_H

#include <stdint.h>

// local hashing + tiny stateful rng for the spawn driver. same splitmix /
// xorshift family worldgen uses, but its own state so reseeding a spawn pass
// can never perturb terrain noise. everything pure on (coords, seed).
//
// yes this is the fourth copy of this in the tree. no i will not be factoring
// it out today, the per-module locality is the point.

uint32_t mspawn_hash2(int x, int z, uint32_t seed);
uint32_t mspawn_hash3(int x, int y, int z, uint32_t seed);

// hashed float in [0,1). dodges the round-to-one trap.
float    mspawn_hash_f01(int x, int y, int z, uint32_t seed);

// fold two seeds into one. used to derive per-pass / per-site seeds.
uint32_t mspawn_seed_mix(uint32_t a, uint32_t b);

typedef struct { uint64_t s; } mspawn_rng;

void     mspawn_rng_seed(mspawn_rng *r, uint32_t seed);
uint32_t mspawn_rng_next(mspawn_rng *r);
int      mspawn_rng_range(mspawn_rng *r, int lo, int hi);   // inclusive
float    mspawn_rng_f01(mspawn_rng *r);
float    mspawn_rng_frange(mspawn_rng *r, float lo, float hi);
// 1 with probability p else 0.
int      mspawn_rng_chance(mspawn_rng *r, float p);

#endif
