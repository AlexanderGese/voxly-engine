#include "effects_modifier.h"
#include "effects_def.h"

#include <stddef.h>

// level == amplifier + 1, since amplifier 0 reads as "level I" in the ui.
static int level_of(const effects_set *s, effects_kind kind) {
    const effects_instance *e = effects_set_find_const(s, kind);
    return e ? e->amplifier + 1 : 0;
}

effects_modifiers effects_modifier_identity(void) {
    effects_modifiers m;
    m.move_speed_mult = 1.0f;
    m.jump_vel_mult = 1.0f;
    m.dig_speed_mult = 1.0f;
    m.melee_damage_mult = 1.0f;
    m.incoming_damage_mult = 1.0f;
    m.absorption_hp = 0;
    m.gamma_floor = 0.0f;
    m.render_alpha = 1.0f;
    m.fire_immune = false;
    m.water_breathing = false;
    m.glowing = false;
    m.levitating = false;
    m.levitation_accel = 0.0f;
    return m;
}

effects_modifiers effects_modifier_compute(const effects_set *s) {
    effects_modifiers m = effects_modifier_identity();
    if (!s) return m;

    // movement. speed/slowness stack onto one multiplier; clamp the low end so
    // slowness VIII doesn't pin you to the floor (vanilla lets it, we won't).
    int spd = level_of(s, EFFECT_SPEED);
    int slo = level_of(s, EFFECT_SLOWNESS);
    const effects_def *d_spd = effects_def_get(EFFECT_SPEED);
    const effects_def *d_slo = effects_def_get(EFFECT_SLOWNESS);
    m.move_speed_mult += d_spd->magnitude * (float)spd;
    m.move_speed_mult -= d_slo->magnitude * (float)slo;
    if (m.move_speed_mult < 0.10f) m.move_speed_mult = 0.10f;

    // jump boost is purely additive per level.
    int jmp = level_of(s, EFFECT_JUMP_BOOST);
    m.jump_vel_mult += effects_def_get(EFFECT_JUMP_BOOST)->magnitude * 0.10f * (float)jmp;

    // dig speed: haste up, fatigue down. fatigue is brutal and multiplicative.
    int haste = level_of(s, EFFECT_HASTE);
    int fatigue = level_of(s, EFFECT_FATIGUE);
    m.dig_speed_mult *= (1.0f + effects_def_get(EFFECT_HASTE)->magnitude * (float)haste);
    for (int i = 0; i < fatigue; i++) m.dig_speed_mult *= 0.30f;

    // combat. strength adds flat-ish damage as a multiplier; weakness subtracts.
    int str = level_of(s, EFFECT_STRENGTH);
    int weak = level_of(s, EFFECT_WEAKNESS);
    m.melee_damage_mult += 0.30f * (float)str;
    m.melee_damage_mult -= 0.20f * (float)weak;
    if (m.melee_damage_mult < 0.0f) m.melee_damage_mult = 0.0f;

    // resistance: each level shaves 20% off incoming, capping at an 80% cut so
    // resistance V isn't literal invincibility.
    int res = level_of(s, EFFECT_RESISTANCE);
    float cut = 0.20f * (float)res;
    if (cut > 0.80f) cut = 0.80f;
    m.incoming_damage_mult = 1.0f - cut;

    // absorption hearts. 4 hp per level (two hearts), the def carries the 4.
    int absn = level_of(s, EFFECT_ABSORPTION);
    m.absorption_hp = (int)(effects_def_get(EFFECT_ABSORPTION)->magnitude) * absn;

    // render + gates.
    if (effects_set_has(s, EFFECT_NIGHT_VISION)) m.gamma_floor = 1.0f;
    if (effects_set_has(s, EFFECT_INVISIBILITY)) m.render_alpha = 0.0f;
    m.fire_immune     = effects_set_has(s, EFFECT_FIRE_RESIST);
    m.water_breathing = effects_set_has(s, EFFECT_WATER_BREATHING);
    m.glowing         = effects_set_has(s, EFFECT_GLOWING);

    int lev = level_of(s, EFFECT_LEVITATION);
    if (lev > 0) {
        m.levitating = true;
        // gentle, scales with level. the def magnitude is the per-level accel.
        m.levitation_accel = effects_def_get(EFFECT_LEVITATION)->magnitude * (float)lev;
    }

    return m;
}

int effects_modifier_haste_level(const effects_set *s) {
    return level_of(s, EFFECT_HASTE);
}

int effects_modifier_fatigue_level(const effects_set *s) {
    return level_of(s, EFFECT_FATIGUE);
}
