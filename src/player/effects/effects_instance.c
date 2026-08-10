#include "effects_instance.h"

#include <stddef.h>

static int clamp_amp(int a) {
    if (a < 0) return 0;
    if (a > EFFECTS_MAX_AMPLIFIER) return EFFECTS_MAX_AMPLIFIER;
    return a;
}

void effects_instance_reset(effects_instance *e) {
    if (!e) return;
    uint16_t gen = e->gen;   // preserve the generation across the wipe
    e->kind = EFFECT_NONE;
    e->amplifier = 0;
    e->duration = 0;
    e->total = 0;
    e->tick_accum = 0;
    e->source_id = 0;
    e->ambient = false;
    e->show_particles = true;
    e->active = false;
    e->gen = gen;
}

void effects_instance_set(effects_instance *e, effects_kind kind, int amplifier,
                          int duration, uint32_t source_id, bool ambient) {
    if (!e) return;
    e->kind = kind;
    e->amplifier = clamp_amp(amplifier);
    e->duration = duration;
    e->total = (duration == EFFECTS_DURATION_INFINITE) ? 1 : duration;
    e->tick_accum = 0;
    e->source_id = source_id;
    e->ambient = ambient;
    // ambient auras render faint particles by default, like a beacon haze.
    e->show_particles = true;
    e->active = true;
    // claiming a slot is the moment a stale handle should stop resolving.
    e->gen++;
}

bool effects_instance_outranks(const effects_instance *a, int amplifier,
                               int duration) {
    amplifier = clamp_amp(amplifier);
    if (amplifier > a->amplifier) return true;
    if (amplifier < a->amplifier) return false;
    // same strength: only an honestly-longer dose is an upgrade. infinite beats
    // everything finite, and nothing beats infinite.
    if (a->duration == EFFECTS_DURATION_INFINITE) return false;
    if (duration == EFFECTS_DURATION_INFINITE) return true;
    return duration > a->duration;
}

bool effects_instance_merge(effects_instance *e, effects_stack_rule rule,
                            int amplifier, int duration, uint32_t source_id) {
    amplifier = clamp_amp(amplifier);
    bool changed = false;

    switch (rule) {
    case EFFECT_STACK_ADD_DUR:
        // durations pile up. amp still only ratchets upward — a level I refill
        // shouldn't downgrade your level II.
        if (amplifier > e->amplifier) { e->amplifier = amplifier; e->tick_accum = 0; changed = true; }
        if (duration == EFFECTS_DURATION_INFINITE || e->duration == EFFECTS_DURATION_INFINITE) {
            if (e->duration != EFFECTS_DURATION_INFINITE) { e->duration = EFFECTS_DURATION_INFINITE; changed = true; }
        } else {
            e->duration += duration;
            e->total += duration;
            changed = true;
        }
        break;

    case EFFECT_STACK_HIGHEST:
        // keep the strongest, never lengthen. absorption hoards its biggest hit.
        if (effects_instance_outranks(e, amplifier, duration)) break;
        if (amplifier > e->amplifier) { e->amplifier = amplifier; e->tick_accum = 0; changed = true; }
        break;

    case EFFECT_STACK_REFRESH:
    default:
        // the common path: an upgrade replaces, a sidegrade-or-worse is ignored,
        // a same-strength-but-longer dose just extends the clock.
        if (amplifier > e->amplifier) {
            e->amplifier = amplifier;
            e->tick_accum = 0;        // recadence at the new (faster) interval
            changed = true;
        }
        if (duration == EFFECTS_DURATION_INFINITE) {
            if (e->duration != EFFECTS_DURATION_INFINITE) { e->duration = EFFECTS_DURATION_INFINITE; changed = true; }
        } else if (e->duration != EFFECTS_DURATION_INFINITE && duration > e->duration) {
            e->duration = duration;
            if (duration > e->total) e->total = duration;
            changed = true;
        }
        break;
    }

    if (changed) e->source_id = source_id;
    return changed;
}

bool effects_instance_expired(const effects_instance *e) {
    if (!e->active) return true;
    if (e->duration == EFFECTS_DURATION_INFINITE) return false;
    return e->duration <= 0;
}

float effects_instance_fraction(const effects_instance *e) {
    if (!e->active) return 0.0f;
    if (e->duration == EFFECTS_DURATION_INFINITE) return 1.0f;
    if (e->total <= 0) return 0.0f;
    float f = (float)e->duration / (float)e->total;
    if (f < 0.0f) f = 0.0f;
    if (f > 1.0f) f = 1.0f;
    return f;
}
