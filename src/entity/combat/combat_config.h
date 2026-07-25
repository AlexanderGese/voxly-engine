#ifndef ENTITY_COMBAT_COMBAT_CONFIG_H
#define ENTITY_COMBAT_COMBAT_CONFIG_H

// combat tunables. kept out of the global config.h so the subsystem owns
// its own knobs
// has plenty too).

// invulnerability window after taking a hit. classic minecraft is 0.5s.
#define COMBAT_IFRAME_TIME      0.5f

// how long the hurt flash / stagger lasts. shorter than iframes on purpose.
#define COMBAT_HURT_TIME        0.35f

// default melee swing cooldown if an attack spec doesnt override it.
#define COMBAT_DEFAULT_COOLDOWN 0.6f

// armor: each point cuts this fraction of incoming damage, capped below.
#define COMBAT_ARMOR_PER_POINT  0.04f   // 4% per point
#define COMBAT_ARMOR_MAX_REDUCE 0.80f   // never block more than 80%

// crit: chance + multiplier. crits ignore the iframe-grace damage clamp.
#define COMBAT_CRIT_CHANCE      0.10f
#define COMBAT_CRIT_MULT        1.5f

// damage variance: rolled damage is scaled by [1-VAR, 1+VAR].
#define COMBAT_DAMAGE_VARIANCE  0.15f

// knockback tuning. horizontal push + a little vertical pop.
#define COMBAT_KB_VERTICAL      0.36f   // fraction of horizontal added to y
#define COMBAT_KB_MIN           0.0f
#define COMBAT_KB_MAX           24.0f   // clamp so explosions dont launch to orbit

// fall damage: damage = (fall_blocks - threshold) * per-block.
#define COMBAT_FALL_THRESHOLD   3.0f
#define COMBAT_FALL_PER_BLOCK   1.0f

// passive regen: heal HEAL_AMOUNT every HEAL_INTERVAL seconds while
// out of combat (no hit for REGEN_GRACE seconds).
#define COMBAT_REGEN_INTERVAL   1.0f
#define COMBAT_REGEN_AMOUNT     1
#define COMBAT_REGEN_GRACE      5.0f

// fire ticks: damage applied every interval while burning.
#define COMBAT_FIRE_INTERVAL    0.5f
#define COMBAT_FIRE_DAMAGE      1

#endif
