#ifndef ENTITY_COMBAT_COMBAT_EXPLOSION_H
#define ENTITY_COMBAT_COMBAT_EXPLOSION_H
#include "combat_types.h"
#include "combat_rng.h"
#include "../../math/vec3.h"
// area-of-effect damage. creepers, tnt. a blast at a point hits everything
// in radius with distance + line-of-sight falloff and big radial knockback.
typedef struct {
    vec3     center;
    float    power;      // damage at the epicenter
    float    radius;     // outer edge, damage 0 here
    uint32_t source_id;  // attribution (the creeper that popped)
} combat_explosion;
// build a blast.
combat_explosion combat_explosion_make(vec3 center, float power, float radius,
                                       uint32_t source_id);
// how much damage this blast deals to something at `pos`, before mitigation.
// uses a squared falloff so it bites hard up close and tapers off. `exposure`
// in [0,1] scales it (1 = clear line of sight, 0 = fully shielded) — the
// caller does the raycast and passes the result.
int combat_explosion_damage_at(const combat_explosion *b, vec3 pos, float exposure);
// apply the blast to one combatant at `target_pos`. fills `out` (may be NULL)
// with the dealt damage + knockback. returns true if it dealt damage.
// `exposure` is the caller-computed visibility fraction.
bool combat_explosion_hit(const combat_explosion *b, combat_combatant *target,
                          vec3 target_pos, float exposure, combat_rng *rng,
                          combat_result *out);
#endif
