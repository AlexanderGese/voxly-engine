#ifndef WORLD_CAVEGEN_NOISE_H
#define WORLD_CAVEGEN_NOISE_H

#include <stdint.h>

// self-contained noise for the cave generator. kept apart from the engine
// perlin on purpose: caves reseed a lot and i dont want a stray noise_seed()
// in here knocking the surface terrain out of alignment. everything is pure
// and deterministic on (coords, seed).

// splitmix-ish integer hashes.
uint32_t cavegen_hash3(int x, int y, int z, uint32_t seed);
uint32_t cavegen_hash1(uint32_t v, uint32_t seed);

// hashed float in [0,1)
float    cavegen_hash_f01(int x, int y, int z, uint32_t seed);

// trilinear value noise, [-1,1], unit grid.
float    cavegen_value3(float x, float y, float z, uint32_t seed);

// fractal value noise. octaves stacked. roughly [-1,1].
float    cavegen_fbm3(float x, float y, float z, uint32_t seed,
                      int octaves, float lacunarity, float gain);

// ridged 3d, [0,1]. nice for worm steering fields.
float    cavegen_ridge3(float x, float y, float z, uint32_t seed, int octaves);

#endif
