#include "effects_controller.h"
#include "effects_def.h"

#include <stddef.h>

void effects_controller_init(effects_controller *ctl) {
    effects_set_init(&ctl->set);
    effects_immunity_clear(&ctl->immunity);
    effects_event_log_init(&ctl->log);
    ctl->tick_accum = 0.0f;
    ctl->cached = effects_modifier_identity();
    ctl->dirty = true;
}

// map an apply result onto the event we log for it. ignored/immune/invalid don't
// produce a transition worth a toast.
static void log_apply(effects_controller *ctl, effects_apply_result res,
                      effects_kind kind, int amplifier) {
    switch (res.result) {
    case EFFECTS_APPLY_ADDED:
        effects_event_push(&ctl->log, EFFECT_EVENT_GAINED, kind, amplifier);
        ctl->dirty = true;
        break;
    case EFFECTS_APPLY_UPGRADED:
        effects_event_push(&ctl->log, EFFECT_EVENT_UPGRADED, kind, amplifier);
        ctl->dirty = true;
        break;
    case EFFECTS_APPLY_REFRESHED:
        effects_event_push(&ctl->log, EFFECT_EVENT_REFRESHED, kind, amplifier);
        // a refresh doesn't change derived numbers, so no dirty flag.
        break;
    default:
        break;
    }
}

effects_apply_result effects_controller_give(effects_controller *ctl,
                                             effects_kind kind, int amplifier,
                                             int duration, uint32_t source_id) {
    effects_apply_result res =
        peffects_apply(&ctl->set, &ctl->immunity, kind, amplifier, duration,
                      source_id, false);
    log_apply(ctl, res, kind, amplifier);
    return res;
}

// snapshot which kinds are live, tick, then diff to discover what expired so we
// can log each expiry. the set itself reaps them; we just observe the gap.
void effects_controller_update(effects_controller *ctl, float dt, float rate,
                               effects_tick_report *out) {
    bool before[EFFECTS_MAX_ACTIVE];
    int  amp_before[EFFECTS_MAX_ACTIVE];
    for (int k = 0; k < EFFECTS_MAX_ACTIVE; k++) {
        const effects_instance *e = effects_set_find_const(&ctl->set, (effects_kind)k);
        before[k] = (e != NULL);
        amp_before[k] = e ? e->amplifier : 0;
    }

    effects_tick_dt(&ctl->set, dt, rate, &ctl->tick_accum, out);

    for (int k = 1; k < EFFECTS_MAX_ACTIVE; k++) {
        bool now = effects_set_has(&ctl->set, (effects_kind)k);
        if (before[k] && !now) {
            effects_event_push(&ctl->log, EFFECT_EVENT_EXPIRED,
                               (effects_kind)k, amp_before[k]);
            ctl->dirty = true;
        }
    }
}

int effects_controller_cure_all(effects_controller *ctl) {
    // gather first so we can log each before the set forgets them.
    effects_kind doomed[EFFECTS_MAX_ACTIVE];
    int amps[EFFECTS_MAX_ACTIVE];
    int n = 0;
    for (int k = 1; k < EFFECTS_MAX_ACTIVE; k++) {
        const effects_instance *e = effects_set_find_const(&ctl->set, (effects_kind)k);
        if (e && effects_def_get(e->kind)->curable) {
            doomed[n] = (effects_kind)k;
            amps[n] = e->amplifier;
            n++;
        }
    }

    int removed = effects_curative_milk(&ctl->set);
    for (int i = 0; i < n; i++) {
        effects_event_push(&ctl->log, EFFECT_EVENT_CURED, doomed[i], amps[i]);
    }
    if (removed > 0) ctl->dirty = true;
    return removed;
}

bool effects_controller_remove(effects_controller *ctl, effects_kind kind) {
    const effects_instance *e = effects_set_find_const(&ctl->set, kind);
    int amp = e ? e->amplifier : 0;
    bool ok = effects_curative_remove(&ctl->set, kind);
    if (ok) {
        effects_event_push(&ctl->log, EFFECT_EVENT_CURED, kind, amp);
        ctl->dirty = true;
    }
    return ok;
}

const effects_modifiers *effects_controller_modifiers(effects_controller *ctl) {
    if (ctl->dirty) {
        ctl->cached = effects_modifier_compute(&ctl->set);
        ctl->dirty = false;
    }
    return &ctl->cached;
}
