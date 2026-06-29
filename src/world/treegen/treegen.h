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
// treegen_bush      hand-built shrubs (live / dead / capped)
// treegen_grass     per-cell ground cover scatter (seam-stable, allocation-free)
// treegen_deco      the pass the worldgen driver calls to decorate a chunk
//
// the whole thing is chunk-free below treegen_deco, so the grammar and geometry
// can be fuzzed without spinning up a world. nothing here touches the renderer.
//
// rough call shape from the worldgen driver:
//
// treegen_deco d;
// treegen_deco_config cfg = treegen_deco_config_default(world_seed);
// treegen_deco_init(&d, &cfg);
// ... for each freshly generated chunk:
// treegen_deco_chunk(&d, chunk, my_surface_fn, my_world);
// ... at shutdown:
// treegen_deco_free(&d);

#include "treegen_types.h"
#include "treegen_rand.h"
#include "treegen_buffer.h"
#include "treegen_turtle.h"
#include "treegen_lsystem.h"
#include "treegen_tree.h"
#include "treegen_bush.h"
#include "treegen_grass.h"
#include "treegen_deco.h"

// quick sanity grow used by the offline self-test (treegen_demo.c) and handy in
// a debugger: grow one plant of `kind` into `out`, return voxels emitted. thin
// wrapper but it keeps the demo from reaching into per-module headers.
int treegen_grow_one(struct treegen_buffer *out, treegen_kind kind, uint32_t seed);

#endif
