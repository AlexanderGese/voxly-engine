#ifndef PLAYER_TOOLS_DROP_H
#define PLAYER_TOOLS_DROP_H

#include "tools_types.h"
#include "tools_item.h"
#include "../../world/block.h"
#include "../../math/rng.h"

// drop resolution: given a broken block and the tool that broke it, decide what
// (if anything) pops out. handles silk touch, fortune, and the "no tool, no
// drop" rule that ores enforce.

#define TOOLS_MAX_DROPS 8

typedef struct {
    block_id id;
    int      count;
} drop_stack;

typedef struct {
    drop_stack item[TOOLS_MAX_DROPS];
    int        count;
} drop_list;

// a static drop rule for a block. base_min..base_max of `drop`, optionally a
// silk-touch override (drop the block itself). fortune scales the count for
// ores. xp is just carried along for whoever wants it.
typedef struct {
    block_id base_drop;       // what the block normally yields
    int      min, max;        // count range
    int      fortune_ore;     // 1 = fortune multiplies the drop (ores)
    int      silk_self;       // 1 = silk touch yields the original block
    int      xp;              // experience awarded
} drop_rule;

void  tools_drop_init(void);

// look up the rule for a block. unregistered blocks default to "drop self, 1".
const drop_rule *tools_drop_rule(block_id block);

void  tools_drop_set(block_id block, block_id drop, int min, int max,
                     int fortune_ore, int silk_self, int xp);

// resolve drops into `out`. returns the xp awarded. respects harvestability:
// if the tool can't harvest the block, `out` comes back empty and xp is 0.
int   tools_drop_resolve(const tool_item *t, block_id block, rng *r, drop_list *out);

#endif
