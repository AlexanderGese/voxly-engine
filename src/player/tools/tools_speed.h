#ifndef PLAYER_TOOLS_SPEED_H
#define PLAYER_TOOLS_SPEED_H

#include "tools_types.h"
#include "tools_item.h"
#include "../../world/block.h"

// the dig math. turns (tool, block, environment) into a break time in seconds.
// modelled on the classic voxel-game formula but tuned to our hardness units.

// environmental modifiers that slow you down. all booleans except depth.
typedef struct {
    int   underwater;     // no aqua-affinity, 5x slower
    int   on_ground;      // airborne digging is 5x slower
    float haste;          // speed buff multiplier, 1.0 == none
    float fatigue;        // mining-fatigue debuff, 1.0 == none, <1 slows
} dig_env;

dig_env tools_env_default(void);

// raw multiplier the tool applies to this block (kind match + tier + efficiency
// + enchants), before environment. >= 1 always.
float   tools_speed_multiplier(const tool_item *t, block_id block);

// can this tool actually harvest the block (right kind if required, right tier
// level)? if not, you can still break it but it drops nothing.
int     tools_speed_can_harvest(const tool_item *t, block_id block);

// final break time in seconds. accounts for tool, block hardness, harvest
// gating (un-harvestable blocks take the 5x "wrong tool" penalty) and env.
// returns 0 for instant-break blocks.
float   tools_speed_break_time(const tool_item *t, block_id block, const dig_env *env);

// progress added per second toward breaking. 1/break_time, clamped. used by
// the mining session to accumulate.
float   tools_speed_per_second(const tool_item *t, block_id block, const dig_env *env);

#endif
