#ifndef PLAYER_TOOLS_LOADOUT_H
#define PLAYER_TOOLS_LOADOUT_H

#include "tools_types.h"
#include "tools_item.h"
#include "../../world/block.h"
#include "../../config.h"

// the player's set of held tools, one per hotbar slot. mostly a convenience
// wrapper so the rest of the engine has a single "what am i holding" object and
// an auto-pick helper for "best tool for this block".

typedef struct {
    tool_item slot[HOTBAR_SLOTS];
    int       has[HOTBAR_SLOTS];     // 1 if the slot holds a real tool
    int       selected;
    tool_item hand;                  // the fallback, always valid
} tools_loadout;

void        tools_loadout_init(tools_loadout *L);

// put a tool in a slot. pass kind=TOOL_HAND to clear the slot.
void        tools_loadout_set(tools_loadout *L, int slot, tool_item t);
void        tools_loadout_clear(tools_loadout *L, int slot);

void        tools_loadout_select(tools_loadout *L, int slot);

// the tool in the selected slot, or the hand if empty/out of range.
tool_item  *tools_loadout_current(tools_loadout *L);

// index of the fastest *usable* tool for a block (best break time, must be able
// to harvest if possible). returns -1 if nothing beats the hand. doesn't change
// the selection, just reports.
int         tools_loadout_best_for(tools_loadout *L, block_id block);

// auto-switch selection to the best tool for a block. returns the new slot, or
// -1 if it left the hand. handy to bind to a "smart mining" toggle.
int         tools_loadout_auto_switch(tools_loadout *L, block_id block);

#endif
