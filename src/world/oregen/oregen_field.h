#ifndef WORLD_OREGEN_FIELD_H
#define WORLD_OREGEN_FIELD_H
#include "oregen_types.h"
// continuous ore-density field. an alternative to the discrete seed+blob
// path: sample a scalar density at any world voxel and threshold it to
// decide ore/no-ore. this is what the "rich vein" biome variant uses, where
// we want smeary nougat-y deposits instead of crisp lumps. the seed+blob
float oregen_field_density(const oregen_ore *ore, int x, int y, int z,
                           uint32_t seed);
int oregen_field_hit(const oregen_ore *ore, int x, int y, int z,
                     float threshold, uint32_t seed);
int oregen_field_pick(int x, int y, int z, float threshold, uint32_t seed);
float oregen_field_estimate_rate(const oregen_ore *ore, float threshold,
                                 int samples, uint32_t seed);
#endif
