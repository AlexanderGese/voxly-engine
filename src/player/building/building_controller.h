#ifndef PLAYER_BUILDING_CONTROLLER_H
#define PLAYER_BUILDING_CONTROLLER_H

#include "building_types.h"
#include "building_history.h"
#include "building_preview.h"
#include "../break_progress.h"
#include "../../math/vec3.h"
#include "../../world/world.h"

// the front door of the building module. the game loop hands this an input
// snapshot once per frame and it drives everything: resolves the target, runs
// break-progress, fires placements on a cooldown, keeps the preview fresh.
// place/break/validate/history are the parts; this is the machine.

// one frame of building-relevant input. the caller fills this from its own
// input layer so we don't drag a keyboard dependency in here.
typedef struct {
    vec3  feet;        // player.pos
    float yaw, pitch;  // look angles, radians
    int   mining;      // LMB held
    int   placing;     // RMB held (edge-detected internally for tap-place)
    block_id hand_id;  // selected hotbar block (BLOCK_AIR if empty hand)
    int   tool_tier;   // current tool tier, for break speed
} building_input;

// what happened this frame, for the caller to react to (sounds, drops, hud).
typedef struct {
    int   broke;          // a block was removed this frame
    block_id broke_id;
    int   broke_drop;     // drop block id from the break
    int   broke_count;    // drop stack size
    int   broke_x, broke_y, broke_z;

    int   placed;         // a block was placed this frame
    block_id placed_id;
    int   placed_x, placed_y, placed_z;

    int   place_fail;     // last BPLACE_* if a place was attempted and refused
} building_event;

typedef struct {
    building_history hist;
    building_preview preview;
    break_progress   breaking;

    building_target  target;   // resolved this frame

    float place_cooldown;      // seconds until the next held-place tick
    int   prev_placing;        // for rising-edge tap detection
    float reach;               // cached PLAYER_REACH

    int   enabled;
} building_controller;

void building_controller_init(building_controller *bc);

// run one frame. fills `out` (may be NULL). returns the resolved target so the
// caller can poke the hud / debug overlay.
building_target building_controller_update(building_controller *bc, world *w,
                                           const building_input *in,
                                           float dt, building_event *out);

// undo/redo passthrough so the caller doesn't reach into the history directly.
int building_controller_undo(building_controller *bc, world *w);
int building_controller_redo(building_controller *bc, world *w);

#endif
