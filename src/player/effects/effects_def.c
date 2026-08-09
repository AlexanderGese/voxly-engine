#include "effects_def.h"

#include <stddef.h>
#include <string.h>

// the big table. designated initializers keyed by kind so reordering the enum
// can't silently shift a row onto the wrong effect. magic numbers everywhere,
//.h owns the gameplay-critical ones, these are flavour.
//
// interval_ticks assumes the caller ticks at 20 hz (the usual game tick), so
// 25 ticks ~= 1.25s, matching the old combat_dot poison cadence.
static const effects_def k_def[EFFECT_KIND_COUNT] = {
    [EFFECT_NONE] = {
        "none", "none", EFFECT_CAT_NEUTRAL, EFFECT_STACK_HIGHEST,
        0, 0, 0, false, false, 0, 0.0f, true, false
    },
    [EFFECT_SPEED] = {
        "speed", "speed", EFFECT_CAT_BENEFICIAL, EFFECT_STACK_REFRESH,
        124, 175, 198, true, false, 0, 0.20f, false, false
    },
    [EFFECT_SLOWNESS] = {
        "slowness", "slowness", EFFECT_CAT_HARMFUL, EFFECT_STACK_REFRESH,
        90, 108, 129, true, false, 0, 0.15f, false, false
    },
    [EFFECT_HASTE] = {
        "haste", "haste", EFFECT_CAT_BENEFICIAL, EFFECT_STACK_REFRESH,
        217, 192, 67, true, false, 0, 0.20f, false, false
    },
    [EFFECT_FATIGUE] = {
        "fatigue", "mining_fatigue", EFFECT_CAT_HARMFUL, EFFECT_STACK_REFRESH,
        74, 66, 23, true, false, 0, 0.30f, false, false
    },
    [EFFECT_STRENGTH] = {
        "strength", "strength", EFFECT_CAT_BENEFICIAL, EFFECT_STACK_REFRESH,
        147, 36, 35, true, false, 0, 3.0f, false, false
    },
    [EFFECT_WEAKNESS] = {
        "weakness", "weakness", EFFECT_CAT_HARMFUL, EFFECT_STACK_REFRESH,
        72, 77, 72, true, false, 0, 4.0f, false, false
    },
    [EFFECT_JUMP_BOOST] = {
        "jump_boost", "jump_boost", EFFECT_CAT_BENEFICIAL, EFFECT_STACK_REFRESH,
        34, 255, 76, true, false, 0, 1.0f, false, false
    },
    [EFFECT_REGENERATION] = {
        // heals on a fast cadence; amplifier makes it faster, not bigger.
        "regeneration", "regeneration", EFFECT_CAT_BENEFICIAL, EFFECT_STACK_REFRESH,
        205, 92, 171, true, true, 50, 1.0f, false, false
    },
    [EFFECT_POISON] = {
        "poison", "poison", EFFECT_CAT_HARMFUL, EFFECT_STACK_REFRESH,
        87, 123, 35, true, true, 25, 1.0f, false, true
    },
    [EFFECT_WITHER] = {
        "wither", "wither", EFFECT_CAT_HARMFUL, EFFECT_STACK_REFRESH,
        53, 42, 39, true, true, 40, 1.0f, false, true
    },
    [EFFECT_FIRE_RESIST] = {
        "fire_resistance", "fire_resistance", EFFECT_CAT_BENEFICIAL, EFFECT_STACK_REFRESH,
        228, 154, 58, true, false, 0, 0.0f, false, false
    },
    [EFFECT_WATER_BREATHING] = {
        "water_breathing", "water_breathing", EFFECT_CAT_BENEFICIAL, EFFECT_STACK_REFRESH,
        152, 218, 192, true, false, 0, 0.0f, false, false
    },
    [EFFECT_NIGHT_VISION] = {
        // cosmetic-ish: a milk bucket leaves it alone.
        "night_vision", "night_vision", EFFECT_CAT_BENEFICIAL, EFFECT_STACK_REFRESH,
        31, 31, 161, false, false, 0, 0.0f, false, false
    },
    [EFFECT_INVISIBILITY] = {
        "invisibility", "invisibility", EFFECT_CAT_BENEFICIAL, EFFECT_STACK_REFRESH,
        127, 131, 146, true, false, 0, 0.0f, false, false
    },
    [EFFECT_RESISTANCE] = {
        "resistance", "resistance", EFFECT_CAT_BENEFICIAL, EFFECT_STACK_REFRESH,
        153, 69, 58, true, false, 0, 0.20f, false, false
    },
    [EFFECT_ABSORPTION] = {
        "absorption", "absorption", EFFECT_CAT_BENEFICIAL, EFFECT_STACK_HIGHEST,
        36, 107, 251, true, false, 0, 4.0f, false, false
    },
    [EFFECT_SATURATION] = {
        "saturation", "saturation", EFFECT_CAT_BENEFICIAL, EFFECT_STACK_ADD_DUR,
        137, 35, 31, true, true, 1, 1.0f, false, false
    },
    [EFFECT_HUNGER] = {
        "hunger", "hunger", EFFECT_CAT_HARMFUL, EFFECT_STACK_REFRESH,
        88, 124, 41, true, false, 0, 0.5f, false, false
    },
    [EFFECT_GLOWING] = {
        "glowing", "glowing", EFFECT_CAT_NEUTRAL, EFFECT_STACK_REFRESH,
        148, 166, 166, false, false, 0, 0.0f, false, false
    },
    [EFFECT_LEVITATION] = {
        "levitation", "levitation", EFFECT_CAT_NEUTRAL, EFFECT_STACK_REFRESH,
        206, 255, 255, true, false, 0, 0.9f, false, false
    },
};

// returned for bogus kinds so callers never deref NULL.
static const effects_def k_sentinel = {
    "?", "?", EFFECT_CAT_NEUTRAL, EFFECT_STACK_HIGHEST,
    0, 0, 0, false, false, 0, 0.0f, true, false
};

const effects_def *effects_def_get(effects_kind kind) {
    if (kind <= EFFECT_NONE || kind >= EFFECT_KIND_COUNT) return &k_sentinel;
    return &k_def[kind];
}

effects_kind effects_def_from_id(const char *id) {
    if (!id) return EFFECT_NONE;
    for (int k = 1; k < EFFECT_KIND_COUNT; k++) {
        if (strcmp(k_def[k].id, id) == 0) return (effects_kind)k;
    }
    return EFFECT_NONE;
}

int effects_def_interval(effects_kind kind, int amplifier) {
    const effects_def *d = effects_def_get(kind);
    if (!d->ticks || d->interval_ticks <= 0) return 0;
    if (amplifier < 0) amplifier = 0;

    int iv = d->interval_ticks;
    // each level halves the gap, but never below one tick or the loop in
    // effects_tick would spin forever draining nothing.
    for (int i = 0; i < amplifier && iv > 1; i++) iv /= 2;
    if (iv < 1) iv = 1;
    return iv;
}

bool effects_def_is_buff(effects_kind kind) {
    return effects_def_get(kind)->category == EFFECT_CAT_BENEFICIAL;
}
