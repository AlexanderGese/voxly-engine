#include "combat_attack.h"
#include "combat_config.h"
#include "combat_invuln.h"

#include <math.h>

// stock weapons. arc_cos ~0.5 => a 120 degree swing cone, generous enough
// to feel forgiving without hitting things behind you.
static const combat_attack k_weapons[COMBAT_WEAPON_COUNT] = {
    [COMBAT_WEAPON_FIST]  = { "fist",  1, COMBAT_DMG_MELEE, 3.0f, 0.30f, 2.0f, 0.5f },
    [COMBAT_WEAPON_SWORD] = { "sword", 6, COMBAT_DMG_MELEE, 3.5f, 0.40f, 3.0f, 0.6f },
    [COMBAT_WEAPON_AXE]   = { "axe",   8, COMBAT_DMG_MELEE, 3.0f, 0.50f, 4.0f, 0.9f },
    [COMBAT_WEAPON_CLAW]  = { "claw",  3, COMBAT_DMG_MELEE, 1.8f, 0.20f, 2.5f, 0.7f },
};

const combat_attack *combat_attack_get(combat_weapon_id id) {
    if (id < 0 || id >= COMBAT_WEAPON_COUNT) id = COMBAT_WEAPON_FIST;
    return &k_weapons[id];
}

bool combat_attack_can_swing(const combat_combatant *attacker) {
    return !attacker->dead && combat_cooldown_ready(attacker);
}

bool combat_attack_in_arc(const combat_attack *a, vec3 origin, vec3 dir,
                          vec3 target_pos) {
    vec3 to = vec3_sub(target_pos, origin);
    to.y = 0.0f;   // melee arc is judged on the flat plane

    float dist = sqrtf(to.x * to.x + to.z * to.z);
    if (dist > a->reach) return false;
    if (dist < 0.0001f) return true;   // basically on top of us, count it

    // normalize the facing too, in case the caller passed something unscaled.
    float dl = sqrtf(dir.x * dir.x + dir.z * dir.z);
    if (dl < 0.0001f) return true;     // no facing, dont gate on the cone

    float dot = (to.x * dir.x + to.z * dir.z) / (dist * dl);
    return dot >= a->arc_cos;
}

combat_hit combat_attack_build(const combat_attack *a, combat_combatant *attacker,
                               vec3 origin, vec3 target_pos) {
    combat_hit h;
    h.amount     = a->damage;
    h.type       = a->type;
    h.source_id  = attacker ? attacker->id : 0;
    h.source_pos = origin;
    h.target_pos = target_pos;
    h.knockback  = a->knockback;
    h.flags      = COMBAT_HIT_NONE;

    // stamp cooldown so this attacker has to wait before the next swing.
    if (attacker) {
        float cd = a->cooldown > 0.0f ? a->cooldown : COMBAT_DEFAULT_COOLDOWN;
        combat_cooldown_start(attacker, cd);
    }
    return h;
}
