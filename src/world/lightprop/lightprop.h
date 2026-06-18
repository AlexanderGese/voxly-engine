#ifndef WORLD_LIGHTPROP_H
#define WORLD_LIGHTPROP_H

#include "lightprop_types.h"

// public entry points. this is what the rest of the engine calls; everything
// else under lightprop/ is plumbing. drop-in nicer cousin of lighting.c:
// incremental instead of nuke-and-pave, with proper removal so torches and
// placed blocks don't leave ghost light.
//
// usage from block_update / the mesher side:
// lightprop_chunk_full(w, c);                  // after worldgen
// lightprop_on_place(w, x, y, z, old, new);    // block changed
// lightprop_on_break(w, x, y, z, old);         // block removed

// full recompute of a freshly generated chunk: sky shafts + flood, plus a sweep
// for any emitters inside it. use this once per chunk, not per edit.
void lightprop_chunk_full(world *w, chunk *c);

// a block at (wx,wy,wz) just changed from `old_id` to `new_id`. handles the four
// interesting transitions (transparent<->opaque, emitter on/off) for both
// channels and repairs the surrounding light.
void lightprop_on_change(world *w, int wx, int wy, int wz,
                         block_id old_id, block_id new_id);

// convenience wrappers around lightprop_on_change.
void lightprop_on_place(world *w, int wx, int wy, int wz,
                        block_id old_id, block_id new_id);
void lightprop_on_break(world *w, int wx, int wy, int wz, block_id old_id);

// emitter brightness for a block id (0 if it doesn't glow). exposed so callers
// can decide whether an edit even touches the block channel.
uint8_t lightprop_emission(block_id id);

#endif
