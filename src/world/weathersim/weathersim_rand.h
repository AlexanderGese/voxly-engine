#ifndef WORLD_WEATHERSIM_RAND_H
#define WORLD_WEATHERSIM_RAND_H

#include <stdint.h>

// self-contained hashing + tiny stateful rng for the weather sim. same
// splitmix/xorshift family the rest of worldgen uses, but local state so the
// front roller can never perturb terrain noise. fronts spawn off hashed
// (cell, time-bucket, seed) tuples so a given world replays the same sky.

uint32_t weathersim_hash2(int x, int z, uint32_t seed);
uint32_t weathersim_hash3(int x, int y, int z, uint32_t seed);

// hashed float in [0,1).
float weathersim_hash_f01(int x, int y, int z, uint32_t seed);

// mix two seeds into one. used to derive per-front seeds.
uint32_t weathersim_seed_mix(uint32_t a, uint32_t b);

// small stateful generator. seed once, then pull. xorshift64*.
typedef struct { uint64_t s; } weathersim_rng;

void     weathersim_rng_seed(weathersim_rng *r, uint32_t seed);
uint32_t weathersim_rng_next(weathersim_rng *r);
int      weathersim_rng_range(weathersim_rng *r, int lo, int hi);  // inclusive
float    weathersim_rng_f01(weathersim_rng *r);
float    weathersim_rng_frange(weathersim_rng *r, float lo, float hi);
// 1 with probability p, else 0.
int      weathersim_rng_chance(weathersim_rng *r, float p);
// gaussian-ish sample, mean 0 stddev ~1. just averages a few uniforms, which
// is plenty for jittering front headings — not doing box-muller for wind.
float    weathersim_rng_gauss(weathersim_rng *r);

#endif
