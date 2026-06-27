#ifndef WORLD_STRONGHOLD_ROOM_H
#define WORLD_STRONGHOLD_ROOM_H

#include "stronghold_graph.h"
#include "stronghold_buffer.h"
#include "stronghold_rand.h"

// room carver. each room is hollowed out of solid stone: a stone-brick shell,
// an air interior, a torch or two for light. then per-type decoration runs:
// junctions get a central pillar, prisons get barred cells, stairwells get a
// ladder column. libraries and the portal room have their own modules because
// they're elaborate enough to warrant it.
//
// note: corridors carve their doorways through these shells afterwards, so the
// shell-then-door order matters; the driver enforces it.

// carve a room shell + interior + lighting. dispatches decoration by type
// (except library/portal which the driver routes to their own builders).
int stronghold_room_carve(const stronghold_room *r, stronghold_buffer *out,
                          stronghold_rng *rng);

// carve every non-library, non-portal room. returns voxels emitted.
int stronghold_room_carve_all(const stronghold_graph *g, stronghold_buffer *out,
                              stronghold_rng *rng);

#endif
