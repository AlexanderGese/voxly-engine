#ifndef ENTITY_COMBAT_COMBAT_KNOCKBACK_H
#define ENTITY_COMBAT_COMBAT_KNOCKBACK_H

#include "combat_types.h"
#include "../../math/vec3.h"

// knockback math. given an attack origin and a target position, work out
// the velocity delta to shove the target away. the resolver applies it.

// horizontal push away from `source`, plus a small upward pop. `strength`
// is the base m/s, scaled by the damage types knockback_mult and clamped.
// returns the velocity delta (add to target vel).
vec3 combat_knockback_dir(vec3 target_pos, vec3 source_pos,
                          float strength, combat_damage_type type);

// radial knockback for explosions: falls off with distance up to `radius`.
// `power` is the strength at the center. used by creeper / tnt.
vec3 combat_knockback_radial(vec3 target_pos, vec3 source_pos,
                             float power, float radius);

// clamp a knockback vector to the configured max magnitude so nothing
// gets launched into orbit.
vec3 combat_knockback_clamp(vec3 kb);

#endif
