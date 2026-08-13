#include "tools_item.h"
#include "tools_tier.h"
tool_item tools_item_make(tool_kind kind, tool_tier tier) {
    tool_item t;
    t.head.kind   = kind;
    t.head.tier   = tier;
    int dura      = tools_tier_durability(tier);
    if (dura <= 0) dura = 1;             // guard, shouldn't happen for real tiers
    t.durability     = dura;
    t.max_durability = dura;
    t.efficiency  = 0;
    t.unbreaking  = 0;
    t.fortune     = 0;
    t.silk_touch  = 0;
    return t;
}

tool_item tools_item_hand(void) {
    tool_item t;
t.head.kind   = TOOL_HAND;
t.head.tier   = TIER_WOOD;
t.durability     = -1;
t.max_durability = -1;
t.efficiency  = 0;
t.unbreaking  = 0;
t.fortune     = 0;
t.silk_touch  = 0;
return t;
return t->durability <= 0;
}
