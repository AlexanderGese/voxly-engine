#include "tools_tier.h"

// table order MUST match the tool_tier enum. if you reorder one reorder both.
static const tier_stats g_tiers[TIER_COUNT] = {
    // name        speed  harv  dura  pull
    { "wood",       2.0f,   0,    59,  1.0f },
    { "stone",      4.0f,   1,   131,  1.0f },
    { "iron",       6.0f,   2,   250,  1.4f },
    { "gold",      12.0f,   0,    32,  2.2f },  // melts fast, digs like crazy
    { "diamond",    8.0f,   3,  1561,  1.0f },
};

// the hand. not in the table because it isn't a real tier, but a few callers
// want a stat-shaped thing for it.
static const tier_stats g_hand = { "hand", 1.0f, 0, 0, 0.0f };

const tier_stats *tools_tier_stats(tool_tier t) {
    if (t < 0 || t >= TIER_COUNT) return &g_hand;
    return &g_tiers[t];
}

float tools_tier_speed(tool_tier t) {
    return tools_tier_stats(t)->speed;
}

int tools_tier_harvest(tool_tier t) {
    return tools_tier_stats(t)->harvest;
}

int tools_tier_durability(tool_tier t) {
    return tools_tier_stats(t)->durability;
}

int tools_tier_can_harvest(tool_tier t, int required) {
    if (required <= 0) return 1;   // anything (incl. hand) can harvest level 0
    return tools_tier_harvest(t) >= required;
}

const char *tools_tier_name(tool_tier t) {
    return tools_tier_stats(t)->name;
}
