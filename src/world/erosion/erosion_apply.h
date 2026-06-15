#ifndef WORLD_EROSION_APPLY_H
#define WORLD_EROSION_APPLY_H

#include <stdint.h>
#include "erosion_types.h"
#include "erosion_field.h"
#include "../chunk.h"

// the only stage that knows what a block is. takes the eroded float heightmap
// and reconciles a chunk's column stack to it: where erosion lowered the
// surface we clear blocks down to the new height; where it raised the surface
// (deposition) we fill the gap with sediment material. we only touch the top
// surface band, never the cave/ore interior the rest of worldgen built.

// pick the block a freshly deposited surface cell should be. sandy near sea
// level, dirt/grass above. deliberately simple, the biome pass can override.
block_id erosion_deposit_block(int world_y, int sea_level);

// reconcile a single column. `old_top` is the surface height before erosion,
// `new_top` after. clears or fills between them. returns the signed number of
// blocks changed (+ filled, - cleared), handy for the stats overlay.
int erosion_apply_column(chunk *c, int lx, int lz,
                         int old_top, int new_top, int sea_level);

// reconcile a whole chunk against the field. `old_height` is the per-column
// pre-erosion surface (PAD-offset, same layout as the field) so we know how
// far down the old grass/dirt went. returns total blocks changed.
int erosion_apply_chunk(chunk *c, const erosion_field *f,
                        const int *old_height, int sea_level);

#endif
