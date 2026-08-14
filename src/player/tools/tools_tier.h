#ifndef PLAYER_TOOLS_TIER_H
#define PLAYER_TOOLS_TIER_H

#include "tools_types.h"

// per-tier stats. mining speed is the multiplier applied when the tool is the
// right kind for the block. harvest level gates whether drops happen at all.

typedef struct {
    const char *name;
    float       speed;        // dig speed multiplier vs hand
    int         harvest;      // harvest level, compared to block requirement
    int         durability;   // base uses before the head wears out
    float       enchant_pull; // how easily it accepts enchants, unused-ish
} tier_stats;

const tier_stats *tools_tier_stats(tool_tier t);

// convenience accessors so call sites don't poke the struct directly.
float tools_tier_speed(tool_tier t);
int   tools_tier_harvest(tool_tier t);
int   tools_tier_durability(tool_tier t);

// can a tool of this tier actually harvest a block needing `required` level?
// hand counts as level 0. gold is a special snowflake, see the .c.
int   tools_tier_can_harvest(tool_tier t, int required);

const char *tools_tier_name(tool_tier t);

#endif
