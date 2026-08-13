#ifndef PLAYER_TOOLS_ITEM_H
#define PLAYER_TOOLS_ITEM_H

#include "tools_types.h"

// a concrete tool the player holds. has a head (kind+tier), current durability,
// and a couple of enchant fields that the speed/durability/drop math reads.

#define TOOL_ENCH_NONE       0
#define TOOL_ENCH_EFFICIENCY 1   // adds flat dig speed
#define TOOL_ENCH_UNBREAKING 2   // chance to skip durability loss
#define TOOL_ENCH_FORTUNE    3   // more drops
#define TOOL_ENCH_SILK_TOUCH 4   // drop the block itself

typedef struct {
    tool_head head;
    int       durability;     // remaining uses; -1 for the hand (infinite)
    int       max_durability;

    // enchant levels, 0 == not present. fortune/silk are mutually exclusive
    // but we don't enforce it here, the drop code resolves the conflict.
    int       efficiency;
    int       unbreaking;
    int       fortune;
    int       silk_touch;
} tool_item;

// build a fresh tool of a given kind+tier at full durability.
tool_item   tools_item_make(tool_kind kind, tool_tier tier);

// the implicit hand. infinite durability, no enchants.
tool_item   tools_item_hand(void);

int         tools_item_is_hand(const tool_item *t);
int         tools_item_is_broken(const tool_item *t);   // durability hit 0

// fraction of durability remaining, 0..1. hand always reads 1.
float       tools_item_dura_frac(const tool_item *t);

// set an enchant level on the tool. clamps to a sane max.
void        tools_item_enchant(tool_item *t, int ench, int level);

#endif
