#ifndef PLAYER_EFFECTS_EFFECTS_APPLY_H
#define PLAYER_EFFECTS_EFFECTS_APPLY_H

#include "effects_set.h"
#include "effects_curative.h"

// the front door. everything that grants an effect — potions, beacons, command
// blocks, ambient auras, getting hit by a poison arrow — funnels through here so
// the stacking rules and immunity checks live in exactly one place.

// what happened to an application. callers (potion code, hud flash) switch on it.
typedef enum {
    EFFECTS_APPLY_ADDED = 0,   // landed in a fresh slot
    EFFECTS_APPLY_UPGRADED,    // merged and changed an existing instance
    EFFECTS_APPLY_REFRESHED,   // existing instance's clock pushed out, same level
    EFFECTS_APPLY_IGNORED,     // weaker/shorter than what's running, dropped
    EFFECTS_APPLY_IMMUNE,      // carrier can't receive this kind
    EFFECTS_APPLY_INVALID,     // bad args (kind/duration)
} effects_apply_kind;

typedef struct {
    effects_apply_kind result;
    effects_handle     handle;   // points at the affected instance (or NONE)
} effects_apply_result;

// the full form. duration in ticks (or EFFECTS_DURATION_INFINITE). `im` may be
// NULL to skip the immunity gate. ambient marks beacon-style low-key effects.
effects_apply_result peffects_apply(effects_set *s, const effects_immunity *im,
                                   effects_kind kind, int amplifier, int duration,
                                   uint32_t source_id, bool ambient);

// the common shorthand: no immunity gate, not ambient, environment source.
effects_apply_result effects_apply_simple(effects_set *s, effects_kind kind,
                                          int amplifier, int duration);

// did the application actually grant or change something the carrier feels?
bool effects_apply_took(effects_apply_kind r);

#endif
