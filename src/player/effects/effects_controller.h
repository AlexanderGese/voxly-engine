#ifndef PLAYER_EFFECTS_EFFECTS_CONTROLLER_H
#define PLAYER_EFFECTS_EFFECTS_CONTROLLER_H

#include "effects_apply.h"
#include "effects_tick.h"
#include "effects_event.h"
#include "effects_modifier.h"

// the carrier-level façade. bundles the active set, this carrier's immunities,
// and the event log, then wires the apply/tick paths into the log so the player
// loop deals with one object instead of five. this is the thing you'd actually
// hang off a player/mob struct.
//
// the lower files stay independent and testable on their own; this one is just
// the convenient glue that knows how they fit together.

typedef struct {
    effects_set        set;
    effects_immunity   immunity;
    effects_event_log  log;

    float tick_accum;        // carries sub-tick time across dt-driven frames
    effects_modifiers cached;// last computed modifier block, refreshed on change
    bool  dirty;             // set when something changed; modifiers recompute
} effects_controller;

// bring a controller up. no immunities by default; call effects_immunity_* on
// ctl->immunity to add them (or assign effects_immunity_undead()).
void effects_controller_init(effects_controller *ctl);

// grant an effect and log the transition. thin wrapper over peffects_apply that
// also pushes the right event and trips the dirty flag.
effects_apply_result effects_controller_give(effects_controller *ctl,
                                             effects_kind kind, int amplifier,
                                             int duration, uint32_t source_id);

// advance by real seconds. drives ticks, reaps expired effects (logging each),
// and recomputes the cached modifiers if anything moved. `rate` is ticks/sec.
// fills `out` with the frame's accumulated heal/damage/hunger if non NULL.
void effects_controller_update(effects_controller *ctl, float dt, float rate,
                               effects_tick_report *out);

// milk-bucket the carrier. logs a cure event per removed effect. returns count.
int  effects_controller_cure_all(effects_controller *ctl);

// targeted removal, logged. returns true if one was present.
bool effects_controller_remove(effects_controller *ctl, effects_kind kind);

// the up-to-date derived modifiers. recomputes lazily when dirty.
const effects_modifiers *effects_controller_modifiers(effects_controller *ctl);

#endif
