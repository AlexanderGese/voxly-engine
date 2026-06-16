#ifndef WORLD_FARMING_TILE_H
#define WORLD_FARMING_TILE_H

#include "../world.h"
#include "farming_types.h"

// farmland tile lifecycle. a tile is born when you till dirt, lives as long as
// it stays hydrated-or-occupied, accumulates trample from being walked on, and
// dies back to dirt when it dries out empty or gets trampled flat.

// trample cap: once a tile soaks up this much trample it reverts to dirt and
// uproots whatever sat on it. jumping on crops is a classic.
#define FARMING_TRAMPLE_CAP   4

// how long bone-dry *and empty* farmland clings on before crumbling to dirt.
// driven off the tile's dry_timer which hydration_step keeps for us.
#define FARMING_REVERT_SECONDS  24.0f

// initialise a freshly tilled tile at world coords. starts dry-ish; watering or
// rain fills it on the next hydration step.
void farming_tile_init(farming_tile *t, int wx, int wy, int wz);

// register a trample event (a player landing on the tile). returns 1 if this
// pushed the tile over the cap and it should revert.
int farming_tile_trample(farming_tile *t);

// step the tile's "should i revert to dirt" decision. takes whether a live crop
// currently sits on it. returns one of the outcomes below.
typedef enum {
    FARMING_TILE_OK = 0,    // nothing to do
    FARMING_TILE_REVERT,    // crumble back to dirt, uproot any crop
} farming_tile_outcome;

farming_tile_outcome farming_tile_decay(const farming_tile *t, int has_crop,
                                        float dt);

// apply a revert to the world: farmland -> dirt, and clear the crop block above
// if one was there. returns the crop block id that was removed (or BLOCK_AIR).
block_id farming_tile_revert_world(world *w, const farming_tile *t);

#endif
