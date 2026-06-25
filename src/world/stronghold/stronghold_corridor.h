#ifndef WORLD_STRONGHOLD_CORRIDOR_H
#define WORLD_STRONGHOLD_CORRIDOR_H

#include "stronghold_graph.h"
#include "stronghold_buffer.h"

// corridor carver. turns graph edges into actual tunnels of stone brick with a
// 1-wide air channel, punching a doorway at each end. corridors are L-shaped
// when the two rooms dont line up on an axis, and grow a short staircase shaft
// when the edge is flagged as a level drop.
//
// the door records get appended to both rooms so the door decorator can later
// dress them (plank door, iron bars, open arch) without re-deriving geometry.

// carve every edge in the graph into out. returns voxels emitted. also fills in
// per-room door records as a side effect.
int stronghold_corridor_carve_all(stronghold_graph *g, stronghold_buffer *out);

// carve one edge. exposed for reuse/testing.
int stronghold_corridor_carve(stronghold_graph *g, int edge_idx, stronghold_buffer *out);

#endif
