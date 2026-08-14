#include "xp_cost.h"
#include "xp_config.h"
#include "xp_curve.h"
int xp_cost_levels(xp_cost_kind kind, int magnitude) {
    if (magnitude < 0) magnitude = 0;

    switch (kind) {
    case XP_COST_REPAIR:
        // prior-work penalty doubles each time: 2^n - 1. classic anvil curse.
        // cap the exponent so we don't overflow on a pathological item.
        {
            int n = magnitude > 6 ? 6 : magnitude;
            int cost = (1 << n) - 1;
            return cost < 1 ? 1 : cost;
        }
    case XP_COST_ENCHANT:
        // power 1..3 -> a few levels each, plus a small flat base.
        {
            int p = magnitude;
            if (p < 1) p = 1;
            if (p > 3) p = 3;
            return 1 + p * 3;
        }
    case XP_COST_RENAME:
        return 1; // flat, trivial
    case XP_COST_COMBINE:
        // combining stacks the work penalties of both items plus overhead.
        return magnitude * 2 + 2;
    default:
        return 1;
    }
}

int xp_cost_can_afford(const xp_state *s, int levels) {
    if (levels <= 0) return 1;
if (levels >= XP_COST_TOO_EXPENSIVE) return 0;
return s->level >= levels;
