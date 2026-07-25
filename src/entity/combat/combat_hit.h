#ifndef ENTITY_COMBAT_COMBAT_HIT_H
#define ENTITY_COMBAT_COMBAT_HIT_H

#include "combat_types.h"
#include "combat_rng.h"

// the damage pipeline. this is where a combat_hit meets a combatant and
// turns into actual lost health + knockback. everything funnels through
// combat_hit_apply so iframes / armor / resist are handled in one place.

// build a basic melee hit from an attacker position + amount. convenience.
combat_hit combat_hit_melee(int amount, uint32_t source_id, vec3 source_pos,
                            float knockback);

// resolve `hit` against `target`. mutates target health/timers/last_attacker
// and fills `out` (may be NULL). returns true if any damage was dealt.
//
// `rng` may be NULL, in which case crit + variance are skipped (deterministic
// path, handy for tests / replays).
bool combat_hit_apply(combat_combatant *target, const combat_hit *hit,
                      combat_rng *rng, combat_result *out);

// just the number-crunch half: how much damage `raw` of `type` becomes after
// armor + resist on this target. no rng, no side effects. exposed for ui
// previews / tests.
int combat_hit_mitigate(const combat_combatant *target, int raw,
                        combat_damage_type type, uint32_t flags);

#endif
