#ifndef WORLD_STRONGHOLD_PORTAL_H
#define WORLD_STRONGHOLD_PORTAL_H

#include "stronghold_graph.h"
#include "stronghold_buffer.h"
#include "stronghold_rand.h"

// portal room builder. the goal of the whole maze. a sunken room with a square
// frame of "portal" blocks (we use brick + a torch ring as a stand-in since the
// palette has no portal block) sitting over a shallow lava-stand-in moat, a
// staircase down into the pit, and a silverfish spawner stand-in in the corner.
//
// exactly one of these exists per stronghold; the driver routes the single
// portal room here.

// carve the portal room. returns voxels emitted. fills the room shell too, so
// the generic room carver should skip the portal room.
int stronghold_portal_carve(const stronghold_room *r, stronghold_buffer *out,
                            stronghold_rng *rng);

// find + carve the portal room in the graph. returns voxels, or 0 if none.
int stronghold_portal_build(const stronghold_graph *g, stronghold_buffer *out,
                            stronghold_rng *rng);

#endif
