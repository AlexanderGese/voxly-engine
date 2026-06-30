#ifndef WORLD_TREEGEN_TREE_H
#define WORLD_TREEGEN_TREE_H

#include "treegen_types.h"
#include "treegen_buffer.h"
#include "treegen_rand.h"

// grows a whole tree: build the grammar, rewrite it, then walk the word with the
// turtle stamping wood and leaf blobs into `out`. plant-local coords, trunk base
// at (0,0,0). returns total voxels emitted (0 if the species was a dud).
//
// `seed` should already be a per-plant seed (treegen_seed_mix of world xz + the
// global seed) so the same spot regrows the same tree across reloads.
int treegen_tree_grow(treegen_buffer *out, treegen_kind kind, uint32_t seed);

// like above but with caller-supplied tunables, for biome reskins / testing.
int treegen_tree_grow_species(treegen_buffer *out, const treegen_species *sp,
                              uint32_t seed);

#endif
