#ifndef PLAYER_BUILDING_PICK_H
#define PLAYER_BUILDING_PICK_H

#include "building_types.h"
#include "../../math/vec3.h"
#include "../../world/world.h"

// "pick block" — the middle-click that sucks the block you're looking at into
// your hand. plus the little family of sampling helpers that go with it:
// reading what's at a target, the canonical (un-rotated) id for the hotbar,
// and the per-tier tool that block wants. none of this writes the world.

// the block currently under a resolved target's crosshair. air if no target.
block_id building_pick_target(world *w, const building_target *t);

// the id that should land in the hotbar when you pick `id`. strips rotation
// variants back to base and maps a couple of "you can't carry that" blocks to
// a sensible stand-in (e.g. grass -> dirt isn't done here; we keep grass).
block_id building_pick_hotbar_id(block_id id);

// can the player pick this block at all? false for air and tile-entities we
// don't model as carryable yet.
int building_pick_allowed(block_id id);

// which tool tier breaks `id` fastest. lets a pick auto-select the matching
// tool slot if the game wants that. returns 0 (hand) when no tool helps.
int building_pick_best_tool(block_id id);

// sample a 3x3x3 neighborhood around a cell, counting solid neighbors. used by
// smart-placement (e.g. auto-orienting fences) and by the pick HUD to show
// "this block is exposed / buried". writes the count, returns the center id.
block_id building_pick_neighborhood(world *w, int x, int y, int z,
                                    int *solid_neighbors);

#endif
