#ifndef ENTITY_MOBS_MOB_SKELETON_H
#define ENTITY_MOBS_MOB_SKELETON_H

#include "mob_common.h"
#include "mob_rng.h"
#include "../../math/vec3.h"

// skeleton: ranged hostile. keeps its distance and looses arrows.
// strafes to stay in a sweet spot band instead of closing to melee.

// out_fire is set to 1 when the skeleton releases an arrow this frame.
// returns the arrow damage (0 unless it fired). caller spawns the projectile.
int voxl_mob_skeleton_update(voxl_mob *m, voxl_mob_rng *r,
                             vec3 player_pos, int *out_fire, float dt);

#endif
