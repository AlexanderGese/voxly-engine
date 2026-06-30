#ifndef WORLD_TREEGEN_RAND_H
#define WORLD_TREEGEN_RAND_H
#include <stdint.h>
// deterministic hashing + a tiny stateful rng, self-contained so reseeding a
// plant can never perturb terrain. pure on (coords, seed). same splitmix family
// as the rest of worldgen so two subsystems agree on what "the seed" means.
uint32_t treegen_hash2(int x, int z, uint32_t seed);
uint32_t treegen_hash3(int x, int y, int z, uint32_t seed);
// hashed float in [0,1). handy for scatter rolls keyed on a world cell.
float treegen_hash_f01(int x, int z, uint32_t seed);
uint32_t treegen_seed_mix(uint32_t a, uint32_t b);
typedef struct { uint64_t s; } treegen_rng;
void     treegen_rng_seed(treegen_rng *r, uint32_t seed);
uint32_t treegen_rng_next(treegen_rng *r);
int      treegen_rng_range(treegen_rng *r, int lo, int hi);
float    treegen_rng_f01(treegen_rng *r);
float    treegen_rng_jitter(treegen_rng *r, float mag);
int      treegen_rng_chance(treegen_rng *r, float p);
#endif
