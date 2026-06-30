#ifndef WORLD_TREEGEN_TREE_H
#define WORLD_TREEGEN_TREE_H
#include "treegen_types.h"
#include "treegen_buffer.h"
#include "treegen_rand.h"
// grows a whole tree: build the grammar, rewrite it, then walk the word with the
// turtle stamping wood and leaf blobs into `out`. plant-local coords, trunk base
// at (0,0,0). returns total voxels emitted (0 if the species was a dud).
int treegen_tree_grow(treegen_buffer *out, treegen_kind kind, uint32_t seed);
int treegen_tree_grow_species(treegen_buffer *out, const treegen_species *sp,
                              uint32_t seed);
#endif
