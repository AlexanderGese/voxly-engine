#ifndef WORLD_TREEGEN_BUSH_H
#define WORLD_TREEGEN_BUSH_H

#include "treegen_types.h"
#include "treegen_buffer.h"

// bushes and shrubs. these are cheap: a squat stem (0-1 wood) and a lumpy leaf
// mass built from two or three offset blobs so they read as rounded rather than
// spherical. shares the buffer/rand machinery with the trees but skips the whole
// l-system since the shape is too small to be worth a grammar.

// a plain leafy bush. height/spread scale with the seed within sane bounds.
// returns voxels emitted, trunk base at (0,0,0).
int treegen_bush_grow(treegen_buffer *out, uint32_t seed);

// a dead/desert bush: a few bare wood twigs fanning up, no leaves. uses leaf id
// only if the palette wants it; defaults to wood-only sticks.
int treegen_bush_grow_dead(treegen_buffer *out, uint32_t seed);

// a berry/flower shrub variant: small green blob with a colored cap block on
// top, handy for biome accents. `cap` is the block placed at the crown.
int treegen_bush_grow_capped(treegen_buffer *out, uint32_t seed, block_id cap);

#endif
