#ifndef WORLD_GEN2_NOISE_H
#define WORLD_GEN2_NOISE_H

#include <stdint.h>

// self-contained value-noise + helpers for the gen2 biome experiments.
// we keep this separate from the engine perlin so reseeding here cant
// wreck global terrain continuity. all pure, all deterministic on (x,z,seed).

// integer hash, splitmix-ish. good enough for placement decisions.
uint32_t voxl_gen2_hash2(int x, int z, uint32_t seed);
uint32_t voxl_gen2_hash3(int x, int y, int z, uint32_t seed);

// hashed float in [0,1)
float voxl_gen2_hash_f01(int x, int z, uint32_t seed);

// smooth value noise in [-1,1], grid is unit-spaced.
float voxl_gen2_value2(float x, float z, uint32_t seed);

// fractal value noise, octaves stacked. returns roughly [-1,1].
float voxl_gen2_fbm2(float x, float z, uint32_t seed,
                     int octaves, float lacunarity, float gain);

// ridged variant, nice for mountains/mesa. returns [0,1].
float voxl_gen2_ridge2(float x, float z, uint32_t seed, int octaves);

#endif
