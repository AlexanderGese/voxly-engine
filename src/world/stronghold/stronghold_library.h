#ifndef WORLD_STRONGHOLD_LIBRARY_H
#define WORLD_STRONGHOLD_LIBRARY_H

#include "stronghold_graph.h"
#include "stronghold_buffer.h"
#include "stronghold_rand.h"

// library room builder. the showpiece. a tall hall lined with wood "bookshelf"
// columns (we fake shelves with wood blocks since the palette has no shelf),
// rows of free-standing stacks down the middle, a cobweb-free plank floor, and
// for the larger ones a second-story walkway reached by a corner ladder.
//
// big enough and fiddly enough that it lives apart from the generic room carver.

// carve a single library room. returns voxels emitted.
int stronghold_library_carve(const stronghold_room *r, stronghold_buffer *out,
                             stronghold_rng *rng);

// carve every library in the graph.
int stronghold_library_carve_all(const stronghold_graph *g, stronghold_buffer *out);

#endif
