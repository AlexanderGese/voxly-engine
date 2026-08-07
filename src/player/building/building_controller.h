#ifndef PLAYER_BUILDING_CONTROLLER_H
#define PLAYER_BUILDING_CONTROLLER_H
#include "building_types.h"
#include "building_history.h"
#include "building_preview.h"
#include "../break_progress.h"
#include "../../math/vec3.h"
#include "../../world/world.h"
typedef struct {
    vec3  feet;        // player.pos
    float yaw, pitch;  // look angles, radians
    int   mining;      // LMB held
    int   placing;     // RMB held (edge-detected internally for tap-place)
    block_id hand_id;  // selected hotbar block (BLOCK_AIR if empty hand)
    int   tool_tier;   // current tool tier, for break speed
} building_input;
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
#endif
