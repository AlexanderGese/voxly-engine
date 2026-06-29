#ifndef WORLD_TREEGEN_H
#define WORLD_TREEGEN_H
// vegetation subsystem umbrella. include this and you get the whole plant stack:
//
// treegen_types     species table, voxel + palette types, the symbol alphabet
// treegen_rand      deterministic hashing + per-plant rng (terrain-isolated)
// treegen_buffer    owned growable voxel buffer, leaf-blob / wood-column helpers
// treegen_turtle    3d turtle with a rodrigues frame + push/pop stack
// treegen_lsystem   stochastic l-system: ruleset table + string rewrite
// treegen_tree      interpret a rewritten word into trunk/branch/leaf voxels
#include "treegen_types.h"
#include "treegen_rand.h"
#include "treegen_buffer.h"
#include "treegen_turtle.h"
#include "treegen_lsystem.h"
#include "treegen_tree.h"
#include "treegen_bush.h"
#include "treegen_grass.h"
#include "treegen_deco.h"
int treegen_grow_one(struct treegen_buffer *out, treegen_kind kind, uint32_t seed);
#endif
