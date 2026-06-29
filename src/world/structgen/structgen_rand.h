#ifndef WORLD_STRUCTGEN_RAND_H
#define WORLD_STRUCTGEN_RAND_H

#include <stdint.h>

// deterministic hashing + a tiny stateful rng. self-contained on purpose so
// reseeding here can never perturb global terrain. everything is pure on its
// (coords, seed) inputs. splitmix-flavored, same family as gen2 noise.

uint32_t structgen_hash2(int x, int z, uint32_t seed);
uint32_t structgen_hash3(int x, int y, int z, uint32_t seed);

// hashed float in [0,1).
float structgen_hash_f01(int x, int z, uint32_t seed);

// mix two seeds into one. used to derive per-structure / per-piece seeds.
uint32_t structgen_seed_mix(uint32_t a, uint32_t b);

// small stateful generator so piece assembly doesnt have to thread coords
// through every call. seed it once per structure, then pull from it.
typedef struct { uint64_t s; } structgen_rng;

void     structgen_rng_seed(structgen_rng *r, uint32_t seed);
uint32_t structgen_rng_next(structgen_rng *r);
int      structgen_rng_range(structgen_rng *r, int lo, int hi);   // inclusive
float    structgen_rng_f01(structgen_rng *r);
// 1 with probability p, else 0.
int      structgen_rng_chance(structgen_rng *r, float p);

#endif
