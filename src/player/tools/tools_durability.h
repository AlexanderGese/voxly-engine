#ifndef PLAYER_TOOLS_DURABILITY_H
#define PLAYER_TOOLS_DURABILITY_H

#include "tools_types.h"
#include "tools_item.h"
#include "../../world/block.h"
#include "../../math/rng.h"

// tool wear. breaking a block costs 1 use, hoeing/whacking plants costs 0,
// and unbreaking gives a chance to dodge the cost entirely.

// how much durability breaking this block should cost with this tool. 0 if the
// block is free (plants, instant blocks) or the tool is the hand.
int   tools_dura_cost(const tool_item *t, block_id block);

// roll unbreaking. returns 1 if the durability loss should be skipped this time.
// uses the engine rng so it's deterministic if you seed it.
int   tools_dura_roll_skip(const tool_item *t, rng *r);

// apply `cost` uses of wear to the tool, honoring unbreaking. returns 1 if the
// tool broke as a result (durability reached 0). hand is a no-op.
int   tools_dura_apply(tool_item *t, int cost, rng *r);

// convenience: wear the tool for breaking one block. returns 1 if it broke.
int   tools_dura_on_break(tool_item *t, block_id block, rng *r);

// repair by combining two tools of the same kind+tier. mutates `dst`, leaves a
// 5% "anvil bonus" like the real recipe. returns 1 on success.
int   tools_dura_combine(tool_item *dst, const tool_item *src);

#endif
