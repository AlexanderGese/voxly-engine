#ifndef ENTITY_COMBAT_COMBAT_COMBO_H
#define ENTITY_COMBAT_COMBAT_COMBO_H

#include "combat_types.h"
#include "combat_attack.h"
#include <stdint.h>
#include <stdbool.h>

// combo / attack-chain state. sits on the attacker. landing swings inside a
// short window builds a chain that scales damage and tightens the next
// cooldown a touch (rewards rhythm). whiffing or stalling drops it.
//
// this rides on top of combat_attack / the cooldown timers — it doesnt
// replace them, it just decides the multiplier and chain length that the
// caller folds into the hit it builds.

// how long after a landed hit the chain stays open before it lapses.
#define COMBAT_COMBO_WINDOW   1.2f
// max links we count; the multiplier flattens out past here.
#define COMBAT_COMBO_MAX      5

typedef struct {
    int      chain;       // current combo length, 0 = idle
    float    window;      // seconds left to continue the chain
    uint32_t last_target; // who we last connected with (switching is fine)
    bool     primed;      // a swing is queued/charging this frame
} combat_combo;

void combat_combo_init(combat_combo *cb);

// the attacker swung and CONNECTED with `target_id`. advances the chain and
// returns the resulting damage multiplier to apply to this hit.
float combat_combo_land(combat_combo *cb, uint32_t target_id);

// the attacker swung and MISSED. breaks the chain (whiffing costs you).
void combat_combo_miss(combat_combo *cb);

// damage multiplier for the chain as it stands right now, no state change.
float combat_combo_mult(const combat_combo *cb);

// cooldown scale for the NEXT swing given the current chain — a tight combo
// shaves a little off the base cooldown, down to a floor. multiply the
// attack's cooldown by this before stamping it.
float combat_combo_cooldown_scale(const combat_combo *cb);

// advance the open window. once it lapses the chain resets. call per frame.
void combat_combo_tick(combat_combo *cb, float dt);

// is there an active chain right now?
bool combat_combo_active(const combat_combo *cb);

#endif
