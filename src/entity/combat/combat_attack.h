#ifndef ENTITY_COMBAT_COMBAT_ATTACK_H
#define ENTITY_COMBAT_COMBAT_ATTACK_H

#include "combat_types.h"
#include "combat_rng.h"
#include "../../math/vec3.h"

// attacks: a reusable description of "a swing". weapons, claws and spells
// all boil down to one of these. the attacker builds a combat_hit from a
// spec + its facing and feeds it to the resolver.

typedef struct {
    const char        *name;
    int                damage;       // base damage before rolls
    combat_damage_type type;
    float              reach;        // max melee distance (m)
    float              arc_cos;      // cos of half the swing arc, for cone test
    float              knockback;    // base knockback
    float              cooldown;     // seconds between swings
} combat_attack;

// a few stock weapon specs so callers dont have to invent numbers.
typedef enum {
    COMBAT_WEAPON_FIST = 0,
    COMBAT_WEAPON_SWORD,
    COMBAT_WEAPON_AXE,
    COMBAT_WEAPON_CLAW,    // mob melee
    COMBAT_WEAPON_COUNT
} combat_weapon_id;

const combat_attack *combat_attack_get(combat_weapon_id id);

// can the attacker swing right now? (cooldown gate)
bool combat_attack_can_swing(const combat_combatant *attacker);

// is `target_pos` inside the attack reach AND swing cone from `origin`
// facing `dir` (unit, flattened)? used to pick who a swing connects with.
bool combat_attack_in_arc(const combat_attack *a, vec3 origin, vec3 dir,
                          vec3 target_pos);

// build the combat_hit for this attack landing on `target_pos`. also stamps
// the attackers cooldown so they cant immediately swing again.
combat_hit combat_attack_build(const combat_attack *a, combat_combatant *attacker,
                               vec3 origin, vec3 target_pos);

#endif
