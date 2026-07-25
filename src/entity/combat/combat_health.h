#ifndef ENTITY_COMBAT_COMBAT_HEALTH_H
#define ENTITY_COMBAT_COMBAT_HEALTH_H

#include "combat_types.h"
#include <stdbool.h>

// health bookkeeping for a combatant: init, clamp, heal, passive regen.
// damage application lives in combat_hit.c, this is the gentle side.

// zero-init and set a sane default resist profile (all 1.0).
void combat_health_init(combat_combatant *c, uint32_t id, int max_health, int armor);

// heal up to max. negative / zero ignored, no-op if dead.
void combat_health_heal(combat_combatant *c, int amount);

// directly set health, clamped to [0,max]. flips dead flag accordingly.
// used by save-load and admin commands, not the normal damage path.
void combat_health_set(combat_combatant *c, int health);

// full heal + clear the dead flag. respawn helper.
void combat_health_revive(combat_combatant *c);

// 0..1 health fraction, handy for bars.
float combat_health_frac(const combat_combatant *c);

bool combat_health_is_full(const combat_combatant *c);
bool combat_health_is_dead(const combat_combatant *c);

// advance passive regen. heals slowly once the combatant has been out of
// combat (no hit) for COMBAT_REGEN_GRACE seconds. call once per frame.
void combat_health_regen_tick(combat_combatant *c, float dt);

#endif
