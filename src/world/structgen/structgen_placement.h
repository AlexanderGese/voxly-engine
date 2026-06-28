#ifndef WORLD_STRUCTGEN_PLACEMENT_H
#define WORLD_STRUCTGEN_PLACEMENT_H

#include "structgen_types.h"

// decides whether a chunk hosts a structure, and what kind. uses a regular
// grid for villages (so they spread out and never crowd) plus per-chunk
// rolls for the cheaper dungeon/ruin. all deterministic on (chunk, world seed).

// villages live on a coarse grid: one candidate cell per region_size chunks,
// jittered within the cell so they dont line up like a checkerboard.
// fills out_cx/out_cz with the chosen chunk and returns 1 if this chunk is it.
int structgen_village_cell(const structgen_config *cfg, int chunk_x, int chunk_z,
                           uint32_t world_seed, int *out_cx, int *out_cz);

// top-level query: given a chunk, what (if anything) spawns here? returns the
// kind and fills site->anchor_x/z (world block coords) + a derived seed.
// ground_y is left 0; the driver sets it from the heightmap before building.
structgen_kind structgen_pick(const structgen_config *cfg,
                              int chunk_x, int chunk_z, uint32_t world_seed,
                              structgen_site *site);

// derive a stable per-structure seed from its anchor + world seed + kind.
uint32_t structgen_site_seed(structgen_kind kind, int wx, int wz, uint32_t world_seed);

#endif
