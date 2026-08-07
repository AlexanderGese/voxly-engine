#ifndef PLAYER_BUILDING_PLACE_H
#define PLAYER_BUILDING_PLACE_H

#include "building_types.h"
#include "building_history.h"
#include "../../math/vec3.h"
#include "../../world/world.h"

// the actual "put a block down" action. validates, rotates, writes to the
// world, marks the chunk dirty, and records the edit for undo.

// fills `out_edit` with the applied edit on success (so the caller can spawn
// effects / decrement the hotbar). returns a BPLACE_* code.
int building_place_block(world *w, building_history *hist,
                         block_id id, const building_target *t,
                         float yaw, vec3 player_feet,
                         building_edit *out_edit);

// mark the chunk owning (wx,wy,wz) dirty plus any neighbor chunk if the edit
// sits on a chunk border (so the seam remeshes). exposed because break uses
// it too.
void building_mark_dirty(world *w, int wx, int wy, int wz);

#endif
