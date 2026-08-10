#ifndef PLAYER_ENCHANT_MODIFIER_H
#define PLAYER_ENCHANT_MODIFIER_H
#include "enchant_types.h"
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
#endif
