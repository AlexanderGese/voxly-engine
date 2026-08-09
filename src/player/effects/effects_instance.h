#ifndef PLAYER_EFFECTS_EFFECTS_INSTANCE_H
#define PLAYER_EFFECTS_EFFECTS_INSTANCE_H

#include "effects_types.h"

// one live effect sitting on a carrier. durations are measured in game ticks,
// not seconds, because the tick loop is the only thing that mutates them and
// floats-of-seconds drift over a long potion. callers convert at the edges.

typedef struct {
    effects_kind kind;
    int  amplifier;      // 0 == level I

    int  duration;       // ticks remaining, or EFFECTS_DURATION_INFINITE
    int  total;          // ticks the effect was applied with (for hud ratio)
    int  tick_accum;     // counts up toward the next periodic tick

    uint32_t source_id;  // who applied it. 0 == environment / ambient.
    bool ambient;        // from a beacon-style aura: weaker particles, no hud nag
    bool show_particles; // honoured by effects_color
    bool active;

    uint16_t gen;        // bumped each time the slot is recycled, for handles
} effects_instance;

// zero a slot out. leaves gen alone so old handles stay invalid.
void effects_instance_reset(effects_instance *e);

// fill a freshly-claimed slot from an application request.
void effects_instance_set(effects_instance *e, effects_kind kind, int amplifier,
                          int duration, uint32_t source_id, bool ambient);

// would `b` (a new application) be an upgrade over the running `a`? used by the
// refresh stacking rule: stronger amp wins, equal amp + longer dur wins.
bool effects_instance_outranks(const effects_instance *a, int amplifier,
                               int duration);

// merge a new application into an existing instance per the kind's stack rule.
// returns true if anything about the instance actually changed.
bool effects_instance_merge(effects_instance *e, effects_stack_rule rule,
                            int amplifier, int duration, uint32_t source_id);

// has this instance run out? infinite ones never have.
bool effects_instance_expired(const effects_instance *e);

// fraction of duration remaining, 0..1. infinite reads as 1. used by the hud
// bar and the fade-out flash near the end.
float effects_instance_fraction(const effects_instance *e);

#endif
