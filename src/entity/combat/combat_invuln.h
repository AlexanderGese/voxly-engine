#ifndef ENTITY_COMBAT_COMBAT_INVULN_H
#define ENTITY_COMBAT_COMBAT_INVULN_H

#include "combat_types.h"
#include <stdbool.h>

// all the per-combatant countdown timers: invuln frames, hurt flash,
// attack cooldown. one tick function drives them so callers cant forget
// half of them.

// is the combatant currently in its invulnerability window?
bool combat_invuln_active(const combat_combatant *c);

// start the iframe window (and the hurt flash). called by the hit resolver
// on a successful hit. wont shorten an already-longer window.
void combat_invuln_trigger(combat_combatant *c);

// would a hit of this type land right now? accounts for iframes, the dead
// flag, god mode, and types that punch through invuln (fire/void).
bool combat_invuln_can_take_hit(const combat_combatant *c, combat_damage_type t);

// is the combatant still flashing/staggered from the last hit?
bool combat_invuln_is_hurt(const combat_combatant *c);

// attack cooldown helpers.
bool combat_cooldown_ready(const combat_combatant *c);
void combat_cooldown_start(combat_combatant *c, float seconds);

// tick every timer down toward 0. call once per frame, before resolving
// new hits so a hit landed this frame still gets its fresh window.
void combat_invuln_tick(combat_combatant *c, float dt);

#endif
