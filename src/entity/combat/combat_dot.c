#include "combat_dot.h"
#include "combat_damagetype.h"

#include <stddef.h>

// per-kind tuning. base interval (seconds) at amplifier 0, and the damage
// each tick deals. higher amplifier halves the interval (twice as fast).
typedef struct {
    float              base_interval;
    int                tick_damage;
    combat_damage_type as_type;   // which damage type the ticks register as
    bool               can_kill;  // poison floors at 1, wither/bleed dont
} dot_def;

static const dot_def k_dot[COMBAT_DOT_KIND_COUNT] = {
    [COMBAT_DOT_POISON] = { 1.25f, 1, COMBAT_DMG_MAGIC, false },
    [COMBAT_DOT_BLEED]  = { 1.00f, 1, COMBAT_DMG_GENERIC, true  },
    [COMBAT_DOT_WITHER] = { 1.00f, 1, COMBAT_DMG_VOID,    true  },
};

static float dot_interval(combat_dot_kind kind, int amplifier) {
    float iv = k_dot[kind].base_interval;
    // each amplifier level speeds it up, but dont go below a quarter second
    // or it turns into a blender.
    for (int i = 0; i < amplifier && iv > 0.25f; i++) iv *= 0.5f;
    if (iv < 0.25f) iv = 0.25f;
    return iv;
}

void combat_dot_init(combat_dot_set *s) {
    for (int i = 0; i < COMBAT_DOT_MAX; i++) {
        s->slots[i].active = false;
    }
}

static combat_dot *dot_find(combat_dot_set *s, combat_dot_kind kind) {
    for (int i = 0; i < COMBAT_DOT_MAX; i++) {
        if (s->slots[i].active && s->slots[i].kind == kind) return &s->slots[i];
    }
    return NULL;
}

static combat_dot *dot_free_slot(combat_dot_set *s) {
    for (int i = 0; i < COMBAT_DOT_MAX; i++) {
        if (!s->slots[i].active) return &s->slots[i];
    }
    return NULL;
}

bool combat_dot_apply(combat_dot_set *s, combat_dot_kind kind, int amplifier,
                      float duration, uint32_t source_id) {
    if (kind < 0 || kind >= COMBAT_DOT_KIND_COUNT) return false;
    if (amplifier < 0) amplifier = 0;
    if (duration <= 0.0f) return false;

    combat_dot *d = dot_find(s, kind);
    if (d) {
        // already running. only upgrade — a stronger or longer dose wins,
        // a weaker re-application is wasted (vanilla rules, basically).
        bool changed = false;
        if (amplifier > d->amplifier) {
            d->amplifier = amplifier;
            d->tick_accum = 0.0f;   // restart cadence at the new speed
            changed = true;
        }
        if (duration > d->remaining) {
            d->remaining = duration;
            changed = true;
        }
        if (changed) d->source_id = source_id;
        return changed;
    }

    d = dot_free_slot(s);
    if (!d) return false;   // all four slots taken, drop it

    d->kind       = kind;
    d->amplifier  = amplifier;
    d->remaining  = duration;
    d->tick_accum = 0.0f;
    d->source_id  = source_id;
    d->active     = true;
    return true;
}

bool combat_dot_has(const combat_dot_set *s, combat_dot_kind kind) {
    for (int i = 0; i < COMBAT_DOT_MAX; i++) {
        if (s->slots[i].active && s->slots[i].kind == kind) return true;
    }
    return false;
}

void combat_dot_clear(combat_dot_set *s) {
    for (int i = 0; i < COMBAT_DOT_MAX; i++) s->slots[i].active = false;
}

bool combat_dot_cure(combat_dot_set *s, combat_dot_kind kind) {
    combat_dot *d = dot_find(s, kind);
    if (!d) return false;
    d->active = false;
    return true;
}

// fire one tick of a single effect through the resolver. returns dealt dmg.
static int dot_emit(combat_combatant *c, const combat_dot *d, combat_rng *rng) {
    const dot_def *def = &k_dot[d->kind];

    // poison never finishes the job — clamp it so it cant drop below 1.
    int amount = def->tick_damage;
    if (!def->can_kill && c->health - amount < 1) {
        amount = c->health - 1;
        if (amount <= 0) return 0;
    }

    combat_hit h;
    h.amount     = amount;
    h.type       = def->as_type;
    h.source_id  = d->source_id;
    h.source_pos = VEC3_ZERO;
    h.target_pos = VEC3_ZERO;
    h.knockback  = 0.0f;
    h.flags      = COMBAT_HIT_NO_KNOCKBACK | COMBAT_HIT_BYPASS_IFRAMES;

    combat_result r;
    combat_hit_apply(c, &h, rng, &r);
    return r.dealt;
}

int combat_dot_tick(combat_combatant *c, combat_dot_set *s, float dt,
                    combat_rng *rng) {
    if (c->dead) { combat_dot_clear(s); return 0; }
    if (dt < 0.0f) dt = 0.0f;

    int total = 0;
    for (int i = 0; i < COMBAT_DOT_MAX; i++) {
        combat_dot *d = &s->slots[i];
        if (!d->active) continue;

        d->remaining -= dt;
        d->tick_accum += dt;

        float iv = dot_interval(d->kind, d->amplifier);
        while (d->tick_accum >= iv) {
            d->tick_accum -= iv;
            total += dot_emit(c, d, rng);
            if (c->dead) { combat_dot_clear(s); return total; }
        }

        if (d->remaining <= 0.0f) d->active = false;
    }
    return total;
}
