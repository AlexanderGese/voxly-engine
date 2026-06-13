#ifndef WORLD_BIOME_NOISE_H
#define WORLD_BIOME_NOISE_H
#include <stdint.h>
// small, self-contained noise for the biome subsystem. yes there is already
// math/noise.h and gen2_noise.h but i want this module to own its own seeding
// so tweaking biome zone sizes cant ripple into terrain or the gen2 sandbox.
// all pure, deterministic on (x,z,seed).
uint32_t biome_hash2(int x, int z, uint32_t seed);
uint32_t biome_hash3(int x, int y, int z, uint32_t seed);
// hashed float in [0,1)
float biome_hash01(int x, int z, uint32_t seed);
// smooth value noise in [-1,1], unit grid spacing.
float biome_value2(float x, float z, uint32_t seed);
// fractal value noise, octaves stacked. roughly [-1,1].
float biome_fbm2(float x, float z, uint32_t seed,
                 int octaves, float lacunarity, float gain);
// domain-warped fbm. warps the input coords by another fbm before sampling,
// which breaks up the obvious grid alignment on big zones. returns ~[-1,1].
float biome_warp_fbm2(float x, float z, uint32_t seed, float warp, int octaves);
#endif
