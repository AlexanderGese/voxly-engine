#ifndef ENTITY_COMBAT_COMBAT_CONFIG_H
#define ENTITY_COMBAT_COMBAT_CONFIG_H
#define COMBAT_IFRAME_TIME      0.5f
#define COMBAT_HURT_TIME        0.35f
#define COMBAT_DEFAULT_COOLDOWN 0.6f
#define COMBAT_ARMOR_PER_POINT  0.04f   // 4% per point
#define COMBAT_ARMOR_MAX_REDUCE 0.80f   // never block more than 80%
#define COMBAT_CRIT_CHANCE      0.10f
#define COMBAT_CRIT_MULT        1.5f
#define COMBAT_DAMAGE_VARIANCE  0.15f
#define COMBAT_KB_VERTICAL      0.36f   // fraction of horizontal added to y
#define COMBAT_KB_MIN           0.0f
#define COMBAT_KB_MAX           24.0f   // clamp so explosions dont launch to orbit
#define COMBAT_FALL_THRESHOLD   3.0f
#define COMBAT_FALL_PER_BLOCK   1.0f
#define COMBAT_REGEN_INTERVAL   1.0f
#define COMBAT_REGEN_AMOUNT     1
#define COMBAT_REGEN_GRACE      5.0f
