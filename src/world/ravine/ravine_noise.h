#ifndef WORLD_RAVINE_NOISE_H
#define WORLD_RAVINE_NOISE_H

#include <stdint.h>

// small value-noise kit for the ravine carver. used to wobble the spline
// heading, ripple the wall faces so they dont read as a perfect plane, and
// dapple the strata boundaries. self-contained for the same reason cavegen's
// is: no global seeding side effects. all pure on (coords, seed).

// trilinear-ish value noise on the xz plane, output [-1,1], unit grid.
float ravine_value2(float x, float z, uint32_t seed);

// fractal version. octaves stacked with lacunarity/gain. roughly [-1,1].
float ravine_fbm2(float x, float z, uint32_t seed,
                  int octaves, float lacunarity, float gain);

// ridged variant, [0,1]. the sharp creases read nicely as canyon-wall erosion.
float ravine_ridge2(float x, float z, uint32_t seed, int octaves);

#endif
