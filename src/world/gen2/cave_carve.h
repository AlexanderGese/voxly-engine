#ifndef WORLD_GEN2_CAVE_CARVE_H
#define WORLD_GEN2_CAVE_CARVE_H

#include <stdint.h>

// 3d-noise cave carving test. a voxel is "carved" (turned to air) when the
// noise density at that point crosses a threshold. classic swiss-cheese
// caves plus a separate "worm" tunnel field for long passages.

// 1 if (wx,wy,wz) should be carved out (air), 0 if it stays solid.
// surface_y lets us keep a solid crust near the top so caves stay buried.
int gen2_cave_is_carved(int wx, int wy, int wz, int surface_y, uint32_t seed);

// the raw cheese density [0,1] at a point, exposed for tuning/tests.
float gen2_cave_density(int wx, int wy, int wz, uint32_t seed);

// long winding tunnels: 1 if this point sits inside a worm.
int gen2_cave_is_tunnel(int wx, int wy, int wz, uint32_t seed);

#endif
