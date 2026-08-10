#ifndef PLAYER_ENCHANT_MODIFIER_H
#define PLAYER_ENCHANT_MODIFIER_H

#include "enchant_types.h"

// the gameplay side: turn an enchant_set into the actual numbers combat,
// mining and movement code want. nothing here touches those subsystems
// directly (keeps this dir link-clean); they pull the aggregated modifier
// struct and apply it themselves.

typedef struct {
    float melee_bonus;      // flat extra melee damage (sharpness)
    float ranged_bonus;     // flat extra bow damage (power)
    float damage_reduction; // fraction 0..0.8 soaked (protection)
    float mine_speed_mult;  // multiplier >= 1 (efficiency, aqua)
    float knockback;        // extra blocks of shove (knockback/punch)
    float fall_reduction;   // fraction of fall damage soaked (feather fall)
    int   fire_seconds;     // seconds of burn applied (fire aspect)
    int   fortune_level;    // drop-roll level (fortune)
    int   thorns_level;     // reflect level
    int   unbreaking_level; // durability save level
    int   respiration_secs; // bonus breath
    int   silk_touch;       // 1 if silk
    int   infinity;         // 1 if arrowless
} enchant_modifier;

// zero out a modifier to the "no enchants" baseline (mults = 1, rest 0).
void enchant_modifier_clear(enchant_modifier *m);

// fold an entire set into the modifier. additive where it makes sense,
// multiplicative for the speed/reduction terms, clamped to sane caps.
void enchant_modifier_accumulate(enchant_modifier *m, const enchant_set *s);

// convenience: clear + accumulate in one shot.
void enchant_modifier_from_set(enchant_modifier *m, const enchant_set *s);

// roll the bonus-drop multiplier from a fortune level. deterministic given
// the rng; returns how many extra copies to drop (0..fortune_level+1).
int  enchant_modifier_fortune_drops(int fortune_level, unsigned roll);

#endif
