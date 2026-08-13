#include "tools_durability.h"
#include "tools_material.h"
#include "tools_registry.h"

int tools_dura_cost(const tool_item *t, block_id block) {
    if (tools_item_is_hand(t)) return 0;       // hands don't wear out
    float hardness = tools_registry_hardness(block);
    if (hardness <= 0.0f) return 0;            // instant/plant blocks are free

    // swords are the odd one out: they take double wear when used to mine
    // anything that isn't cloth/leaves. discourages using them as picks.
    if (t->head.kind == TOOL_SWORD) {
        mat_class cls = tools_registry_class(block);
        if (cls != MAT_CLOTH && cls != MAT_LEAVES) return 2;
    }
    return 1;
}

int tools_dura_roll_skip(const tool_item *t, rng *r) {
    if (t->unbreaking <= 0) return 0;
    // chance to skip = unbreaking / (unbreaking + 1). standard formula.
    int lvl = t->unbreaking;
    float chance = (float)lvl / (float)(lvl + 1);
    return rng_float01(r) < chance;
}

int tools_dura_apply(tool_item *t, int cost, rng *r) {
    if (tools_item_is_hand(t) || cost <= 0) return 0;

    // each point of cost is rolled independently against unbreaking.
    for (int i = 0; i < cost; i++) {
        if (r && tools_dura_roll_skip(t, r)) continue;
        t->durability -= 1;
        if (t->durability <= 0) {
            t->durability = 0;
            return 1;                          // snapped
        }
    }
    return 0;
}

int tools_dura_on_break(tool_item *t, block_id block, rng *r) {
    int cost = tools_dura_cost(t, block);
    return tools_dura_apply(t, cost, r);
}

int tools_dura_combine(tool_item *dst, const tool_item *src) {
    if (tools_item_is_hand(dst) || tools_item_is_hand(src)) return 0;
    if (dst->head.kind != src->head.kind) return 0;
    if (dst->head.tier != src->head.tier) return 0;
    if (dst->max_durability <= 0) return 0;

    // combine remaining uses plus a 5% bonus, capped at max.
    int bonus = dst->max_durability / 20;      // ~5%
    int total = dst->durability + src->durability + bonus;
    if (total > dst->max_durability) total = dst->max_durability;
    dst->durability = total;

    // enchants merge by taking the higher level of each. cheap and good enough.
    if (src->efficiency > dst->efficiency) dst->efficiency = src->efficiency;
    if (src->unbreaking > dst->unbreaking) dst->unbreaking = src->unbreaking;
    if (src->fortune    > dst->fortune)    dst->fortune    = src->fortune;
    if (src->silk_touch > dst->silk_touch) dst->silk_touch = src->silk_touch;
    if (dst->fortune && dst->silk_touch)   dst->fortune = 0;  // silk wins ties
    return 1;
}
