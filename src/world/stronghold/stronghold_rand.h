#ifndef WORLD_STRONGHOLD_RAND_H
#define WORLD_STRONGHOLD_RAND_H
#include <stdint.h>
uint32_t stronghold_hash2(int x, int z, uint32_t seed);
uint32_t stronghold_hash3(int x, int y, int z, uint32_t seed);
float stronghold_hash_f01(int x, int z, uint32_t seed);
uint32_t stronghold_seed_mix(uint32_t a, uint32_t b);
typedef struct { uint64_t s; } stronghold_rng;
void     stronghold_rng_seed(stronghold_rng *r, uint32_t seed);
uint32_t stronghold_rng_next(stronghold_rng *r);
int      stronghold_rng_range(stronghold_rng *r, int lo, int hi);
float    stronghold_rng_f01(stronghold_rng *r);
#endif
