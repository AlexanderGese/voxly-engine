#ifndef PLAYER_TOOLS_NAME_H
#define PLAYER_TOOLS_NAME_H

#include "tools_types.h"
#include "tools_item.h"

// display strings for tools. the ui pulls these for the hotbar tooltip and the
// held-item label. nothing fancy, just glues tier + kind names together into a
// small static buffer. not thread safe, but neither is the rest of the engine.

const char *tools_name_kind(tool_kind kind);
const char *tools_name_tier(tool_tier tier);

// full name like "iron pickaxe". writes into an internal rotating buffer so you
// can call it a couple times in one printf without clobbering yourself.
const char *tools_name_full(const tool_item *t);

// a one-line condition word based on remaining durability: "pristine", "worn",
// "battered", "about to snap". the hand returns "bare".
const char *tools_name_condition(const tool_item *t);

// enchant suffix string, e.g. " (eff III, unbr II)". empty string if vanilla.
// also writes into a rotating buffer.
const char *tools_name_enchants(const tool_item *t);

#endif
