#ifndef WORLD_LIGHTPROP_ACCESS_H
#define WORLD_LIGHTPROP_ACCESS_H
#include "lightprop_types.h"
// thin world-space light accessors. the propagator works in world coords and
// crosses chunk borders constantly, so it wants get/set that resolve the chunk
// for it. world.h gives us getters but only chunk.h has the setters, hence the
// wrappers. all of these no-op silently on unloaded chunks (border of the
// loaded region) which is exactly the behaviour the bfs wants.
uint8_t lp_get_light(world *w, lp_channel ch, int wx, int wy, int wz);
void    lp_set_light(world *w, lp_channel ch, int wx, int wy, int wz, uint8_t v);
// block id, world space. returns BLOCK_AIR-equivalent (and treats as opaque) for
// chunks that aren't loaded so light doesn't leak off the edge of the world.
block_id lp_get_block(world *w, int wx, int wy, int wz);
// is this cell loaded? used to decide whether to even bother queuing a node.
int lp_cell_loaded(world *w, int wx, int wz);
// mark the chunk owning (wx,wz) dirty so the mesher repaints it. cheap and
// idempotent.
void lp_mark_dirty(world *w, int wx, int wz);
#endif
