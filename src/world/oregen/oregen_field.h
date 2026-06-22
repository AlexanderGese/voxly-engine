#ifndef WORLD_OREGEN_FIELD_H
#define WORLD_OREGEN_FIELD_H

#include "oregen_types.h"

// continuous ore-density field. an alternative to the discrete seed+blob
// path: sample a scalar density at any world voxel and threshold it to
// decide ore/no-ore. this is what the "rich vein" biome variant uses, where
// we want smeary nougat-y deposits instead of crisp lumps. the seed+blob
// path stays the default; this is opt-in.
//
// density(x,y,z) = band_weight(ore, y) * fbm_mask(x,y,z) gated by a global
// rarity. all pure, no rng state.

// raw density at a voxel for one ore kind, in [0,1]. combines the vertical
// curve with a fractal mask so deposits cluster in 3d.
float oregen_field_density(const oregen_ore *ore, int x, int y, int z,
                           uint32_t seed);

// 1 if the field says "place ore here" for this kind, else 0. threshold is
// the ore's rarity knob: higher rarity -> higher threshold -> less ore.
int oregen_field_hit(const oregen_ore *ore, int x, int y, int z,
                     float threshold, uint32_t seed);

// pick the dominant ore kind at a voxel: the highest-density kind whose
// field clears the threshold, or -1 for plain stone. lets one pass place a
// whole layered deposit without running each ore separately.
int oregen_field_pick(int x, int y, int z, float threshold, uint32_t seed);

// estimate the placement fraction of a kind over its band by monte-carlo
// sampling. slow, debug-only, used to calibrate thresholds against the
// discrete path's output.
float oregen_field_estimate_rate(const oregen_ore *ore, float threshold,
                                 int samples, uint32_t seed);

#endif
