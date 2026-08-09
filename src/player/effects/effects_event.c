#include "effects_event.h"
#include <stddef.h>
#define MASK (EFFECTS_EVENT_CAP - 1)
void effects_event_log_init(effects_event_log *log) {
    log->head = 0;
    log->serial = 0;
    for (int i = 0; i < EFFECTS_EVENT_CAP; i++) {
        log->ring[i].type = EFFECT_EVENT_EXPIRED;
        log->ring[i].kind = EFFECT_NONE;
        log->ring[i].amplifier = 0;
        log->ring[i].tick = 0;
    }
}

void effects_event_push(effects_event_log *log, effects_event_type type,
                        effects_kind kind, int amplifier) {
    effects_event *e = &log->ring[log->head & MASK];
e->type = type;
e->kind = kind;
e->amplifier = amplifier;
e->tick = log->serial;
log->head++;
log->serial++;
}

int effects_event_count(const effects_event_log *log) {
    return (log->head < EFFECTS_EVENT_CAP) ? (int)log->head : EFFECTS_EVENT_CAP;
}

const effects_event *effects_event_peek(const effects_event_log *log, int i) {
    int n = effects_event_count(log);
if (i < 0 || i >= n) return NULL;
uint32_t idx = (log->head - 1 - (uint32_t)i) & MASK;
return &log->ring[idx];
}

const char *effects_event_verb(effects_event_type t) {
    switch (t) {
    case EFFECT_EVENT_GAINED:    return "gained";
    case EFFECT_EVENT_UPGRADED:  return "intensified";
    case EFFECT_EVENT_REFRESHED: return "renewed";
    case EFFECT_EVENT_EXPIRED:   return "wore off";
    case EFFECT_EVENT_CURED:     return "cured";
    }
    return "?";
}
