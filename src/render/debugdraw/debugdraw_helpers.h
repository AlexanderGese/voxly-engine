#ifndef RENDER_DEBUGDRAW_HELPERS_H
#define RENDER_DEBUGDRAW_HELPERS_H

#include "debugdraw.h"
#include "../../world/chunk.h"
#include "../../entity/entity.h"

// engine-aware convenience wrappers. these know about chunks, entities and
// world coords so call sites stay short. all built on the core primitives,
// nothing special on the gpu.

// outline a chunk column. cheap is full height. color defaults by whether
// the chunk is dirty (orange) vs clean (faint blue) when c==0.
void debugdraw_chunk_bounds(debugdraw *dd, const chunk *ch, ddcolor c);

// outline a single block cell at world block coords.
void debugdraw_block(debugdraw *dd, int bx, int by, int bz, ddcolor c);

// entity collider + a little facing arrow from yaw. labels with hp if a
// text sink is attached.
void debugdraw_entity(debugdraw *dd, const entity *e, ddcolor c);

// a ground grid of `half` cells either side of center, snapped to integers.
// spacing in blocks. handy for orienting yourself in an empty world.
void debugdraw_grid(debugdraw *dd, vec3 center, int half, float spacing,
                    ddcolor c);

// velocity vector from a position, scaled so it reads at typical speeds.
void debugdraw_velocity(debugdraw *dd, vec3 pos, vec3 vel, ddcolor c);

#endif
