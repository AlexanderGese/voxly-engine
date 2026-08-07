#ifndef PLAYER_BUILDING_REACH_H
#define PLAYER_BUILDING_REACH_H

#include "building_types.h"
#include "../../math/vec3.h"
#include "../../world/world.h"

// reach geometry. target.c does the raycast but it never actually clamps to
// the player's arm length, and "distance" there is measured to the block
// center which over-counts on glancing hits. this file owns the honest reach
// math so both place and break agree on what's in range.
//
// the eye sits PLAYER_EYE_HEIGHT above the feet. most callers have feet (the
// player.pos convention) and a yaw/pitch, so we hand them an eye + forward.

// eye position from feet. just feet + (0, eye_height, 0).
vec3 building_eye_from_feet(vec3 feet);

// forward look vector from yaw/pitch (radians). matches the camera basis:
// yaw rotates around +y, pitch tilts. returns a unit vector.
vec3 building_forward_from_angles(float yaw, float pitch);

// closest point on the block cell (x,y,z) to the eye. used to measure reach
// to the *surface* rather than the center, which is what feels right when you
// shave a far block at an angle.
vec3 building_cell_nearest(vec3 eye, int x, int y, int z);

// surface-accurate distance from eye to a target's hit cell.
float building_reach_to_hit(const building_target *t, vec3 eye);

// is the target within `reach` of the eye? false for invalid targets.
int building_in_reach(const building_target *t, vec3 eye, float reach);

// resolve + reach-clamp in one go. if the raycast lands past `reach` (the
// raycaster may overshoot because it measures along the ray, not to surface)
// the target is invalidated. this is the call place/break should use.
building_target building_reach_resolve(world *w, vec3 eye, vec3 forward,
                                       float reach);

#endif
