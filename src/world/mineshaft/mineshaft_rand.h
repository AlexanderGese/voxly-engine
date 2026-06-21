#ifndef WORLD_MINESHAFT_RAND_H
#define WORLD_MINESHAFT_RAND_H

#include <stdint.h>

// deterministic hashing + a small stateful rng, self-contained so reseeding
// here can never perturb global terrain. same splitmix family the rest of the
// worldgen leans on, just local state. everything is pure on its inputs.

uint32_t mineshaft_hash2(int x, int z, uint32_t seed);
uint32_t mineshaft_hash3(int x, int y, int z, uint32_t seed);

// hashed float in [0,1). handy for per-block decoration decisions that need to
// stay stable across regen (cobweb placement, ore scatter).
float mineshaft_hash_f01(int x, int y, int z, uint32_t seed);

// fold two seeds into one. used to derive per-cell / per-pass seeds.
uint32_t mineshaft_seed_mix(uint32_t a, uint32_t b);

// tiny stateful generator so the maze walk doesnt thread coords everywhere.
typedef struct { uint64_t s; } mineshaft_rng;

void     mineshaft_rng_seed(mineshaft_rng *r, uint32_t seed);
uint32_t mineshaft_rng_next(mineshaft_rng *r);
int      mineshaft_rng_range(mineshaft_rng *r, int lo, int hi);   // inclusive
float    mineshaft_rng_f01(mineshaft_rng *r);
int      mineshaft_rng_chance(mineshaft_rng *r, float p);

// shuffle an int array in place (fisher-yates). the maze uses it to randomize
// which neighbour direction it tries first.
void     mineshaft_rng_shuffle(mineshaft_rng *r, int *a, int n);

#endif
