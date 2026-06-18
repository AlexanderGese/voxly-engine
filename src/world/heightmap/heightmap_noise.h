#ifndef WORLD_HEIGHTMAP_NOISE_H
#define WORLD_HEIGHTMAP_NOISE_H

#include <stdint.h>

// the noise floor for the spline terrain. self-contained on purpose: the
// continentalness/erosion fields need their own seeded hash so they dont
// correlate with the biome climate noise next door. value noise, fbm and a
// ridged variant. nothing fancy, but it has to be cheap because we evaluate
// it a few times per world column and there are a lot of columns.

uint32_t heightmap_hash2(int x, int z, uint32_t seed);
float    heightmap_hash01(int x, int z, uint32_t seed);   // [0,1)

// raw 2d value noise in [-1,1], lattice at integer coords.
float heightmap_value2(float x, float z, uint32_t seed);

// fractal brownian motion, normalised back into [-1,1]. octaves stacked.
float heightmap_fbm2(float x, float z, uint32_t seed,
                     int octaves, float lacunarity, float gain);

// ridged multifractal-ish: 1-|fbm|, squared, mapped to [-1,1]. spikes up
// where the base noise crosses zero, which is what peaks want.
float heightmap_ridged2(float x, float z, uint32_t seed,
                        int octaves, float lacunarity, float gain);

// domain warp. shoves the sample point around by a second noise field so the
// continents stop looking like a regular grid of blobs.
float heightmap_warp2(float x, float z, uint32_t seed, float warp,
                      int octaves);

#endif
