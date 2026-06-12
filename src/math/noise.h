#ifndef MATH_NOISE_H
#define MATH_NOISE_H

// perlin noise + fbm. seeded.
// based on ken perlin's reference impl, simplified

void  noise_seed(unsigned int seed);
float noise2d(float x, float y);
float noise3d(float x, float y, float z);

// fractal brownian motion. octaves stacked noise2d
float noise_fbm2d(float x, float y, int octaves, float lacunarity, float gain);

#endif
