#ifndef WORLD_OREGEN_SEED_H
#define WORLD_OREGEN_SEED_H

#include "oregen_types.h"

// vein seeding. given a chunk-sized column region and the world seed, roll
// where each ore kind tries to plant a vein. this is purely the "where and
// how big" decision; actually growing the blob is oregen_blob's job.
//
// the seeding is deterministic on (chunk coords, ore, world seed) so the
// same chunk always produces the same veins regardless of load order. we
// fold the curve weight into the per-attempt accept roll, which is what
// makes the depth bands feel soft at the edges.

// derive the seed used for a given ore kind in a given chunk. exposed so
// other passes can reproduce the exact same rolls if they need to.
uint32_t oregen_seed_for(int chunk_x, int chunk_z, int kind, uint32_t world_seed);

// fill out[] with seeded veins for one chunk. region is the 16-wide column
// rooted at world (origin_x, origin_z). max_y clamps spawns so we never
// seed above the actual terrain top (pass the column's surface height).
// returns the number of veins written, never more than out_cap.
int oregen_seed_chunk(oregen_vein *out, int out_cap,
                      int origin_x, int origin_z, int max_y,
                      uint32_t world_seed);

// roll the parameters (center, size, radius, shape) for a single vein of
// kind k whose seed point is (sx,sy,sz). pulled out so tests can poke it.
oregen_vein oregen_seed_roll(int kind, int sx, int sy, int sz, uint32_t seed);

#endif
