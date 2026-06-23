#ifndef WORLD_RAVINE_RAND_H
#define WORLD_RAVINE_RAND_H

#include <stdint.h>

// private rng + integer hashes for the ravine carver. same reasoning as
// cavegen: we reseed off chunk coords constantly and dont want a stray global
// noise_seed() in here yanking the surface terrain out of phase. everything is
// pure and deterministic on its inputs.

typedef struct { uint32_t s; } ravine_rng;

void     ravine_rng_seed(ravine_rng *r, uint32_t seed);
uint32_t ravine_rng_u32(ravine_rng *r);
float    ravine_rng_f01(ravine_rng *r);             // [0,1)
float    ravine_rng_frange(ravine_rng *r, float lo, float hi);
int      ravine_rng_range(ravine_rng *r, int lo, int hi); // inclusive

// splitmix-flavoured integer hashes. deterministic, no state.
uint32_t ravine_hash2(int x, int z, uint32_t seed);
uint32_t ravine_hash1(uint32_t v, uint32_t seed);
float    ravine_hash_f01(int x, int z, uint32_t seed);

#endif
