#ifndef WORLD_FARMING_RNG_H
#define WORLD_FARMING_RNG_H

#include <stdint.h>

// a small deterministic prng + spatial hash for the farming subsystem. yes,
// the engine already has math/rng.h, but growth rolls want to be reproducible
// from (world_seed, block coords, tick) without threading a stateful rng
// through every call, so we keep a stateless hash here and a thin stream rng
// on top. same trick oregen pulled.

typedef struct { uint64_t s; } farming_rng;

// stateless: hash a block + a salt into a uniform u32. use this when you want
// the same answer every time you ask about the same tile.
uint32_t farming_hash3(int x, int y, int z, uint32_t seed);
float    farming_hash_f01(int x, int y, int z, uint32_t seed);

// stateful stream, seed it once and pull a run of values.
void     farming_rng_seed(farming_rng *r, uint32_t seed);
uint32_t farming_rng_next(farming_rng *r);
float    farming_rng_f01(farming_rng *r);
int      farming_rng_range(farming_rng *r, int lo, int hi);   // inclusive
int      farming_rng_chance(farming_rng *r, float p);

#endif
