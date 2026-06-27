#ifndef WORLD_STRONGHOLD_RAND_H
#define WORLD_STRONGHOLD_RAND_H

#include <stdint.h>

// deterministic hashing + a tiny stateful rng. self-contained so reseeding
// here can never perturb global terrain. splitmix-flavored, same family as
// structgen so the two subsystems stay byte-compatible where they touch.

uint32_t stronghold_hash2(int x, int z, uint32_t seed);
uint32_t stronghold_hash3(int x, int y, int z, uint32_t seed);

// hashed float in [0,1).
float stronghold_hash_f01(int x, int z, uint32_t seed);

// mix two seeds into one. used to derive per-room / per-piece seeds.
uint32_t stronghold_seed_mix(uint32_t a, uint32_t b);

// small stateful generator so room assembly doesnt thread coords everywhere.
typedef struct { uint64_t s; } stronghold_rng;

void     stronghold_rng_seed(stronghold_rng *r, uint32_t seed);
uint32_t stronghold_rng_next(stronghold_rng *r);
int      stronghold_rng_range(stronghold_rng *r, int lo, int hi);   // inclusive
float    stronghold_rng_f01(stronghold_rng *r);
int      stronghold_rng_chance(stronghold_rng *r, float p);
// pick a weighted index from `weights[0..n)`. returns 0 if all weights zero.
int      stronghold_rng_weighted(stronghold_rng *r, const int *weights, int n);

#endif
