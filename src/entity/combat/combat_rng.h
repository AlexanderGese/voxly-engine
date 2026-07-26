#ifndef ENTITY_COMBAT_COMBAT_RNG_H
#define ENTITY_COMBAT_COMBAT_RNG_H

#include <stdint.h>
#include <stdbool.h>

// little prng wrapper for combat rolls (crit, variance). i didnt want to
// thread the global rng through every hit so combat keeps its own stream.
// same xorshift64* the rest of the engine uses, just local.

typedef struct { uint64_t s; } combat_rng;

// seed it. a 0 seed is remapped so we never get a stuck-at-zero state.
void  combat_rng_seed(combat_rng *r, uint64_t seed);

// raw 64-bit draw.
uint64_t combat_rng_next(combat_rng *r);

// uniform float in [0,1).
float combat_rng_unit(combat_rng *r);

// roll a probability. p<=0 -> false, p>=1 -> true.
bool  combat_rng_chance(combat_rng *r, float p);

// uniform float in [lo,hi].
float combat_rng_range(combat_rng *r, float lo, float hi);

#endif
