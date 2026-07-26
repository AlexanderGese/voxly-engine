#include "combat_knockback.h"
#include "combat_config.h"
#include "combat_damagetype.h"

#include <math.h>

vec3 combat_knockback_dir(vec3 target_pos, vec3 source_pos,
                          float strength, combat_damage_type type) {
    const combat_dmg_info *info = combat_dmg_info_get(type);
    if (!info->does_knockback || strength <= 0.0f) return VEC3_ZERO;

    strength *= info->knockback_mult;

    // flat direction away from the attacker.
    vec3 away = vec3_sub(target_pos, source_pos);
    away.y = 0.0f;
    float len = sqrtf(away.x * away.x + away.z * away.z);

    if (len < 0.0001f) {
        // attacker stood right on top of us. pick a deterministic-ish
        // direction off the low bits of the position so it isnt always +x.
        int bias = ((int)(target_pos.x * 7.0f + target_pos.z * 13.0f)) & 3;
        switch (bias) {
            case 0: away = (vec3){ 1, 0, 0 }; break;
            case 1: away = (vec3){-1, 0, 0 }; break;
            case 2: away = (vec3){ 0, 0, 1 }; break;
            default:away = (vec3){ 0, 0,-1 }; break;
        }
        len = 1.0f;
    }

    vec3 kb;
    kb.x = away.x / len * strength;
    kb.z = away.z / len * strength;
    kb.y = strength * COMBAT_KB_VERTICAL;   // the little pop upward
    return combat_knockback_clamp(kb);
}

vec3 combat_knockback_radial(vec3 target_pos, vec3 source_pos,
                             float power, float radius) {
    if (power <= 0.0f || radius <= 0.0f) return VEC3_ZERO;

    vec3 d = vec3_sub(target_pos, source_pos);
    float dist = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
    if (dist > radius) return VEC3_ZERO;

    // linear falloff. dead center gets full power, edge gets ~0.
    float falloff = 1.0f - (dist / radius);

    vec3 dir;
    if (dist < 0.0001f) {
        dir = VEC3_UP;   // straight up if we are at the blast center
    } else {
        dir = vec3_scale(d, 1.0f / dist);
        dir.y += 0.5f;   // explosions throw you up and out
    }

    vec3 kb = vec3_scale(dir, power * falloff);
    return combat_knockback_clamp(kb);
}

vec3 combat_knockback_clamp(vec3 kb) {
    float mag = sqrtf(kb.x * kb.x + kb.y * kb.y + kb.z * kb.z);
    if (mag > COMBAT_KB_MAX && mag > 0.0001f) {
        float s = COMBAT_KB_MAX / mag;
        kb.x *= s; kb.y *= s; kb.z *= s;
    }
    return kb;
}
