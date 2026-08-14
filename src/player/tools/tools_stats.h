#ifndef PLAYER_TOOLS_STATS_H
#define PLAYER_TOOLS_STATS_H

#include "tools_types.h"
#include "tools_item.h"
#include "../../world/block.h"

// lightweight per-session mining telemetry. not gameplay-critical, but the
// debug overlay and the stats screen read it, and it's handy for tuning the
// hardness numbers. one global instance, reset on world load.

typedef struct {
    long  blocks_broken;                 // total blocks mined this session
    long  by_material[MAT_CLASS_COUNT];  // broken, bucketed by material
    long  by_tool[TOOL_KIND_COUNT];      // broken, bucketed by tool used
    long  durability_spent;              // total tool uses consumed
    long  tools_snapped;                 // tools that broke mid-dig
    long  ore_xp;                        // xp from ores
    float seconds_digging;               // accumulated dig time
} tools_stats;

void  tools_stats_reset(tools_stats *s);

// record a finished break. pass the tool that did it (post-resolve, pre-wear is
// fine), the block, and how much durability the swing cost.
void  tools_stats_on_break(tools_stats *s, const tool_item *t, block_id block,
                           int dura_cost, int snapped, int xp);

// accumulate active dig time, called each tick the player is actually mining.
void  tools_stats_tick(tools_stats *s, float dt);

// average durability spent per block, for the tuning readout. 0 if nothing yet.
float tools_stats_wear_rate(const tools_stats *s);

// most-mined material this session, or MAT_NONE if idle.
mat_class tools_stats_top_material(const tools_stats *s);

#endif
