#ifndef PLAYER_FISHING_RNG_H
#define PLAYER_FISHING_RNG_H

#include <stdint.h>

// the fishing module keeps its own little prng so a cast is reproducible from
// a seed without disturbing the shared world rng. splitmix64, same lineage as
// the oregen one. plenty good for picking fish.

typedef struct { uint64_t s; } fishing_rng;

void     fishing_rng_seed(fishing_rng *r, uint64_t seed);
uint64_t fishing_rng_u64(fishing_rng *r);

// uniform float in [0,1).
float    fishing_rng_float(fishing_rng *r);

// uniform float in [lo,hi).
float    fishing_rng_frange(fishing_rng *r, float lo, float hi);

// inclusive integer in [lo,hi]. lo>hi just returns lo, no drama.
int      fishing_rng_range(fishing_rng *r, int lo, int hi);

// roll a probability. p<=0 -> 0, p>=1 -> 1.
int      fishing_rng_chance(fishing_rng *r, float p);

#endif
