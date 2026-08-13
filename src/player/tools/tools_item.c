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
    t.head.tier   = TIER_WOOD;           // tier is irrelevant for the hand
    t.durability     = -1;
    t.max_durability = -1;
    t.efficiency  = 0;
    t.unbreaking  = 0;
    t.fortune     = 0;
    t.silk_touch  = 0;
    return t;
}

int tools_item_is_hand(const tool_item *t) {
    return t->head.kind == TOOL_HAND;
}

int tools_item_is_broken(const tool_item *t) {
    if (tools_item_is_hand(t)) return 0;  // hand never breaks
    return t->durability <= 0;
}

float tools_item_dura_frac(const tool_item *t) {
    if (tools_item_is_hand(t) || t->max_durability <= 0) return 1.0f;
    float f = (float)t->durability / (float)t->max_durability;
    if (f < 0.0f) f = 0.0f;
    if (f > 1.0f) f = 1.0f;
    return f;
}

void tools_item_enchant(tool_item *t, int ench, int level) {
    if (level < 0) level = 0;
    switch (ench) {
        case TOOL_ENCH_EFFICIENCY: if (level > 5) level = 5; t->efficiency = level; break;
        case TOOL_ENCH_UNBREAKING: if (level > 3) level = 3; t->unbreaking = level; break;
        case TOOL_ENCH_FORTUNE:    if (level > 3) level = 3; t->fortune    = level;
                                   if (level) t->silk_touch = 0; break;
        case TOOL_ENCH_SILK_TOUCH: t->silk_touch = level ? 1 : 0;
                                   if (level) t->fortune = 0; break;
        default: break;
    }
}
