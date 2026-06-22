#ifndef WORLD_OREGEN_NOISE_H
#define WORLD_OREGEN_NOISE_H

#include <stdint.h>

// little 3d value-noise field used to warp vein surfaces so blobs read as
// rough mineral lumps instead of math ellipsoids. self-contained, same hash
// family as oregen_rand, kept apart from the engine perlin so it can never
// nudge terrain. all pure on (x,y,z,seed).

// smooth value noise in [-1,1], unit-spaced lattice.
float oregen_noise3(float x, float y, float z, uint32_t seed);

// fractal stack of the above. octaves of detail, lacunarity grows the
// frequency, gain shrinks the amplitude. returns roughly [-1,1].
float oregen_noise_fbm3(float x, float y, float z, uint32_t seed,
                        int octaves, float lacunarity, float gain);

// turbulence: sum of |noise|, gives the ropey cauliflower look that reads
// well on ore surfaces. returns [0,1]-ish.
float oregen_noise_turb3(float x, float y, float z, uint32_t seed, int octaves);

// signed surface offset to add to a blob radius at direction (dx,dy,dz)
// from center. magnitude scales with amp. this is the one the blob builder
// actually calls.
float oregen_noise_surface(int cx, int cy, int cz, int dx, int dy, int dz,
                           float amp, uint32_t seed);

#endif
