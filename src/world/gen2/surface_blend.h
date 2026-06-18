#ifndef WORLD_GEN2_SURFACE_BLEND_H
#define WORLD_GEN2_SURFACE_BLEND_H

#include <stdint.h>

// biome borders look bad if heights jump. these blend the registry height
// across a small neighborhood so adjacent biomes meet smoothly.

// average the registry height over a (2*radius+1) square around (wx,wz).
// distance-weighted so the center dominates. radius clamped to [0,4].
int gen2_blend_height(int wx, int wz, int sea_level, uint32_t seed, int radius);

// blend factor 0..1: how mixed this column is with a different biome nearby.
// 0 == deep inside one biome, 1 == right on a border. handy for transitions.
float gen2_blend_edge_factor(int wx, int wz, int sea_level, uint32_t seed);

#endif
