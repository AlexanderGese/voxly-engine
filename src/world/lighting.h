#ifndef WORLD_LIGHTING_H
#define WORLD_LIGHTING_H

#include "world.h"

// flood fill based lighting. called after worldgen and on block changes.
// sunlight drops from the sky and is scaled by opacity.
// block light comes from emissive blocks (torches).

void lighting_recompute_chunk(world *w, chunk *c);
void lighting_update_block(world *w, int wx, int wy, int wz);

#endif
