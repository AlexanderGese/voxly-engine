#ifndef WORLD_RIVERS_RAND_H
#define WORLD_RIVERS_RAND_H

#include <stdint.h>

// self-contained hashing + a tiny stateful rng for the river carver. same
// splitmix/xorshift family the rest of worldgen leans on, but kept local so
// reseeding here can never perturb terrain, biome or cave noise. everything
// pure on its (coords, seed) inputs unless you go through the stateful rng.

uint32_t rivers_hash2(int x, int z, uint32_t seed);
uint32_t rivers_hash3(int x, int y, int z, uint32_t seed);

// hashed float in [0,1). cheap, deterministic, used for rain jitter and the
// tie-break wobble that keeps perfectly diagonal slopes from looking like
// staircases.
float rivers_hash_f01(int x, int z, uint32_t seed);

// mix two seeds into one. used to derive per-pass / per-region sub-seeds so
// each stage of the pipeline gets its own stream.
uint32_t rivers_seed_mix(uint32_t a, uint32_t b);

// small stateful generator. seed once, then pull. handy for the outlet notch
// jitter where threading coords through everything would be a pain.
typedef struct { uint64_t s; } rivers_rng;

void     rivers_rng_seed(rivers_rng *r, uint32_t seed);
uint32_t rivers_rng_next(rivers_rng *r);
int      rivers_rng_range(rivers_rng *r, int lo, int hi);   // inclusive
float    rivers_rng_f01(rivers_rng *r);
float    rivers_rng_frange(rivers_rng *r, float lo, float hi);
int      rivers_rng_chance(rivers_rng *r, float p);         // 1 w/ prob p

#endif
