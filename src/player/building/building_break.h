#ifndef PLAYER_BUILDING_BREAK_H
#define PLAYER_BUILDING_BREAK_H

#include "building_types.h"
#include "building_history.h"
#include "../../math/vec3.h"
#include "../../world/world.h"

// the "remove a block" action. validates indestructibility, writes air,
// marks dirty, records undo, and reports what dropped.

// describes what a break yields, so the caller can spawn dropped_items.
typedef struct {
    block_id drop_id;
    int      drop_count;
    vec3     drop_pos;   // world-space spawn point (block center)
} building_drop;

// break the block under the target. fills out_drop + out_edit on success.
// returns a BBREAK_* code.
int building_break_block(world *w, building_history *hist,
                         const building_target *t,
                         building_drop *out_drop,
                         building_edit *out_edit);

// is this block id breakable by hand at all? false for bedrock.
int building_is_breakable(block_id id);

// how long (seconds) the block takes to break with the given tool tier.
// tool_tier 0=hand. mirrors block_ext break_time, scaled by tool match.
float building_break_seconds(block_id id, int tool_tier);

#endif
