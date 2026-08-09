#ifndef PLAYER_EFFECTS_EFFECTS_EVENT_H
#define PLAYER_EFFECTS_EFFECTS_EVENT_H

#include "effects_types.h"

// a tiny fixed ring of "something happened to an effect" records. the hud uses
// it for toast notifications ("Poison wore off"), and it's handy when debugging
// a stacking bug to see the last N transitions without a logger.
//
// own little ring rather than util/ringbuf so it stays trivially copyable for
// save snapshots and doesn't drag a dependency in. the engine has three of
// these already; one more won't hurt.

typedef enum {
    EFFECT_EVENT_GAINED = 0,   // landed fresh
    EFFECT_EVENT_UPGRADED,     // level went up
    EFFECT_EVENT_REFRESHED,    // duration extended, same level
    EFFECT_EVENT_EXPIRED,      // ran out on its own
    EFFECT_EVENT_CURED,        // removed by milk / antidote / command
} effects_event_type;

typedef struct {
    effects_event_type type;
    effects_kind       kind;
    int   amplifier;     // level at the moment of the event
    uint32_t tick;       // monotonically-increasing serial, not a wall clock
} effects_event;

#define EFFECTS_EVENT_CAP 16   // power of two so the mask wraps cleanly

typedef struct {
    effects_event ring[EFFECTS_EVENT_CAP];
    uint32_t head;       // next write slot (monotonic, masked on access)
    uint32_t serial;     // bumped per push, stamped into each event
} effects_event_log;

void effects_event_log_init(effects_event_log *log);

// record one event. overwrites the oldest when full (ring semantics).
void effects_event_push(effects_event_log *log, effects_event_type type,
                        effects_kind kind, int amplifier);

// how many events are currently held (saturates at the capacity).
int  effects_event_count(const effects_event_log *log);

// fetch the i-th newest event (0 == most recent). NULL if i is out of range.
const effects_event *effects_event_peek(const effects_event_log *log, int i);

// short human label for an event type, for toast text.
const char *effects_event_verb(effects_event_type t);

#endif
