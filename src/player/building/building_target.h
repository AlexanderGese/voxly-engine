#ifndef PLAYER_BUILDING_TARGET_H
#define PLAYER_BUILDING_TARGET_H
#include "building_types.h"
#include "../../math/vec3.h"
#include "../../world/world.h"
#include "../raycast.h"
// resolves where the player is pointing into a building_target: the block
// under the crosshair plus the adjacent cell a placement would occupy.
// thin wrapper over raycast_blocks, but it folds in reach clamping and the
// face->adjacent math so place/break code stays clean.
// run the cast and resolve. `origin` is the eye, `dir` the look direction.
building_target building_target_resolve(world *w, vec3 origin, vec3 dir,
                                        float reach);
building_target building_target_from_eye(world *w, vec3 eye, vec3 forward,
                                         float reach);
int building_target_has_block(const building_target *t);
float building_target_distance(const building_target *t, vec3 eye);
#endif
