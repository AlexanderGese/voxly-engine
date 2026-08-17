#ifndef UI_HUD2_HURT_H
#define UI_HUD2_HURT_H

#include "../../math/vec3.h"

// helpers for turning "something hurt me, over there" into the single screen
// angle the vignette wants. kept separate from the vignette so the geometry
// math isnt tangled up with the drawing. all pure, no state.

// project a world-space source position onto the player's view plane and
// return the screen-space hit angle in radians (-pi..pi, 0 = dead ahead,
// positive = to the right / clockwise). yaw is the player's facing in radians.
float hud2_hurt_dir_from_world(vec3 player_pos, float yaw, vec3 source_pos);

// map a raw damage amount (in half-hearts, matching the survival system) to a
// 0..1 flash strength with a soft knee so chip damage still shows but big hits
// dont blow past full.
float hud2_hurt_strength(int damage);

// fall damage / drowning etc have no direction; this picks a "from below"
// angle so the flash reads as environmental rather than an attacker.
float hud2_hurt_dir_environment(void);

#endif
