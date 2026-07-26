#ifndef ENTITY_COMBAT_COMBAT_TYPES_H
#define ENTITY_COMBAT_COMBAT_TYPES_H
#include "../../math/vec3.h"
#include <stdint.h>
#include <stdbool.h>
typedef enum {
    COMBAT_DMG_GENERIC = 0,  // unspecified, no resist
    COMBAT_DMG_MELEE,        // swords, claws, punches
    COMBAT_DMG_PROJECTILE,   // arrows, thrown stuff
    COMBAT_DMG_FALL,         // hit the ground too hard
    COMBAT_DMG_FIRE,         // burning / lava, ticks over time
    COMBAT_DMG_EXPLOSION,    // creeper / tnt, big knockback
    COMBAT_DMG_DROWN,        // out of air
    COMBAT_DMG_VOID,         // fell out of the world, unblockable
    COMBAT_DMG_MAGIC,        // potions etc, ignores armor
    COMBAT_DMG_COUNT
} combat_damage_type;
typedef struct {
    int                 amount;     // raw damage before resist / variance
    combat_damage_type  type;
    uint32_t            source_id;  // who dealt it. 0 = environment.
    vec3                source_pos; // origin, used for knockback direction
    vec3                target_pos; // where the victim is, for knockback dir
    float               knockback;  // base knockback strength (m/s)
    uint32_t            flags;       // COMBAT_HIT_*
} combat_hit;
typedef struct {
    int      dealt;      // damage actually subtracted after all modifiers
    bool     blocked;    // fully absorbed (iframes / dead / immune)
    bool     lethal;     // dropped target to 0
    bool     crit;       // rolled a crit
    vec3     knock;      // velocity delta to apply to the target
} combat_result;
typedef struct {
    uint32_t id;

    int   health;
    int   max_health;
    int   armor;            // flat damage reduction points, 0..20
    float resist[COMBAT_DMG_COUNT]; // per-type multiplier, 1.0 = normal

    // timers, all in seconds, count down to 0.
    float iframe_timer;     // invulnerable while > 0
    float hurt_timer;       // red-flash / stagger window
    float attack_cd;        // cant swing again until 0
    float regen_timer;      // accumulates toward passive regen tick

    // last hit bookkeeping, handy for ai + death attribution.
    uint32_t           last_attacker;
    combat_damage_type last_dmg_type;

    bool  dead;
    bool  invulnerable;     // god mode toggle (creative, cutscenes)
} combat_combatant;
#endif
