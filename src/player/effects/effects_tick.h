#ifndef PLAYER_EFFECTS_EFFECTS_TICK_H
#define PLAYER_EFFECTS_EFFECTS_TICK_H

#include "effects_set.h"

// the heartbeat. advances every active effect by a whole number of game ticks,
// fires periodic effects on their interval, and rolls the side-effects up into
// one struct the host applies to its real health/hunger state.
//
// we deliberately don't reach into the combatant here — this module has no idea
// what a combat_combatant is and we'd like to keep it that way. the host reads
// the report and pokes its own systems. (the apply layer already tagged the
// damaging kinds via overrides_dot if the host prefers to route those through
// combat instead of eating the numbers below.)

// everything one frame of ticking wants to hand back. all amounts are positive
// magnitudes; the field name carries the sign convention.
typedef struct {
    int   heal;          // hp to add (regeneration)
    int   damage;        // hp to subtract (poison, wither)
    int   wither_damage; // of `damage`, how much ignores the 1-hp floor
    float hunger_gain;   // saturation refill, in hunger points
    float hunger_drain;  // hunger to burn (hunger effect)
    int   expired_count; // effects that ran out this frame
    int   tick_events;   // periodic ticks that fired this frame
} effects_tick_report;

// advance the whole set by `ticks` game ticks (usually 1). fills `out` if non
// NULL. expired effects are reaped. ticks <= 0 is a no-op.
void effects_tick(effects_set *s, int ticks, effects_tick_report *out);

// the seconds-facing convenience for hosts that think in dt. accumulates the
// fractional remainder internally via *acc so we never drop or double-count a
// tick across frames. rate is ticks-per-second (20 for the default loop).
void effects_tick_dt(effects_set *s, float dt, float rate, float *acc,
                     effects_tick_report *out);

// zero a report. handy for callers that sum across several carriers.
void effects_tick_report_reset(effects_tick_report *r);

#endif
