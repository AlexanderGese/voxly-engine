#include "effects_tick.h"
#include "effects_def.h"

#include <stddef.h>

void effects_tick_report_reset(effects_tick_report *r) {
    if (!r) return;
    r->heal = 0;
    r->damage = 0;
    r->wither_damage = 0;
    r->hunger_gain = 0.0f;
    r->hunger_drain = 0.0f;
    r->expired_count = 0;
    r->tick_events = 0;
}

// fire one periodic pulse of a single effect into the report. magnitude in the
// def is per-pulse; amplifier scales the steady ones (hunger) but the healing /
// poison pulses stay flat and just come faster, like vanilla.
static void emit_pulse(const effects_instance *e, effects_tick_report *out) {
    const effects_def *d = effects_def_get(e->kind);
    out->tick_events++;

    switch (e->kind) {
    case EFFECT_REGENERATION:
        out->heal += (int)d->magnitude;
        break;
    case EFFECT_POISON:
        // poison's chip is in the report; the host floors it at 1 hp.
        out->damage += (int)d->magnitude;
        break;
    case EFFECT_WITHER:
        out->damage += (int)d->magnitude;
        out->wither_damage += (int)d->magnitude;
        break;
    case EFFECT_SATURATION:
        out->hunger_gain += d->magnitude * (float)(e->amplifier + 1);
        break;
    default:
        // any other ticking kind that sneaks in just burns its event harmlessly.
        break;
    }
}

// advance a single instance by one tick. returns true if it's still alive.
static bool step_one(effects_instance *e, effects_tick_report *out) {
    const effects_def *d = effects_def_get(e->kind);

    // steady-state debuffs that drain over wall time rather than on an interval.
    if (e->kind == EFFECT_HUNGER) {
        out->hunger_drain += d->magnitude * (float)(e->amplifier + 1) / 20.0f;
    }

    if (d->ticks) {
        int iv = effects_def_interval(e->kind, e->amplifier);
        if (iv > 0) {
            e->tick_accum++;
            // a while-loop, not an if, so a level bump mid-life that shrinks the
            // interval still drains a backlog instead of stalling.
            while (e->tick_accum >= iv) {
                e->tick_accum -= iv;
                emit_pulse(e, out);
            }
        }
    }

    if (e->duration == EFFECTS_DURATION_INFINITE) return true;
    e->duration--;
    return e->duration > 0;
}

void effects_tick(effects_set *s, int ticks, effects_tick_report *out) {
    effects_tick_report local;
    if (!out) { out = &local; }
    effects_tick_report_reset(out);

    if (ticks <= 0) return;

    for (int t = 0; t < ticks; t++) {
        for (int i = 0; i < EFFECTS_MAX_ACTIVE; i++) {
            effects_instance *e = &s->slots[i];
            if (!e->active) continue;

            bool alive = step_one(e, out);
            if (!alive) {
                out->expired_count++;
                effects_set_drop(s, e->kind);
            }
        }
    }
}

void effects_tick_dt(effects_set *s, float dt, float rate, float *acc,
                     effects_tick_report *out) {
    if (out) effects_tick_report_reset(out);
    if (dt <= 0.0f || rate <= 0.0f) return;

    float carried = acc ? *acc : 0.0f;
    carried += dt * rate;

    // pull whole ticks out of the accumulator, leave the remainder for next time.
    int whole = (int)carried;
    if (whole < 0) whole = 0;
    carried -= (float)whole;
    if (acc) *acc = carried;

    if (whole > 0) effects_tick(s, whole, out);
}
