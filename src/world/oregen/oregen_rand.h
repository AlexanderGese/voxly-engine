#ifndef WORLD_OREGEN_RAND_H
#define WORLD_OREGEN_RAND_H

#include <stdint.h>

// self-contained hashing + tiny stateful rng for the ore generator. same
// splitmix/xorshift family the rest of worldgen uses, but local state so
// reseeding here can never perturb terrain or biome noise. everything pure
// on its (coords, seed) inputs.

uint32_t oregen_hash2(int x, int z, uint32_t seed);
uint32_t oregen_hash3(int x, int y, int z, uint32_t seed);

// hashed float in [0,1).
float oregen_hash_f01(int x, int y, int z, uint32_t seed);

// mix two seeds into one. used to derive per-ore / per-vein seeds.
uint32_t oregen_seed_mix(uint32_t a, uint32_t b);

// small stateful generator so the blob walkers dont have to thread coords
// through every call. seed once per vein, then pull from it.
typedef struct { uint64_t s; } oregen_rng;

void     oregen_rng_seed(oregen_rng *r, uint32_t seed);
uint32_t oregen_rng_next(oregen_rng *r);
int      oregen_rng_range(oregen_rng *r, int lo, int hi);   // inclusive
float    oregen_rng_f01(oregen_rng *r);
float    oregen_rng_frange(oregen_rng *r, float lo, float hi);
// 1 with probability p, else 0.
int      oregen_rng_chance(oregen_rng *r, float p);
// pick a unit-ish direction component in {-1,0,1}, biased toward nonzero.
int      oregen_rng_step(oregen_rng *r);

#endif
