#include "effects_set.h"

#include <stddef.h>

void effects_set_init(effects_set *s) {
    for (int i = 0; i < EFFECTS_MAX_ACTIVE; i++) {
        s->slots[i].gen = 0;
        effects_instance_reset(&s->slots[i]);
    }
    s->count = 0;
    s->dirty_bits = 0;
}

int effects_set_count(const effects_set *s) {
    return s->count;
}

effects_instance *effects_set_find(effects_set *s, effects_kind kind) {
    if (kind <= EFFECT_NONE) return NULL;
    for (int i = 0; i < EFFECTS_MAX_ACTIVE; i++) {
        if (s->slots[i].active && s->slots[i].kind == kind) return &s->slots[i];
    }
    return NULL;
}

const effects_instance *effects_set_find_const(const effects_set *s, effects_kind kind) {
    if (kind <= EFFECT_NONE) return NULL;
    for (int i = 0; i < EFFECTS_MAX_ACTIVE; i++) {
        if (s->slots[i].active && s->slots[i].kind == kind) return &s->slots[i];
    }
    return NULL;
}

bool effects_set_has(const effects_set *s, effects_kind kind) {
    return effects_set_find_const(s, kind) != NULL;
}

// mark slot i dirty. capped at 64 slots, which the enum is comfortably under.
static void set_dirty(effects_set *s, int i) {
    if (i >= 0 && i < 64) s->dirty_bits |= ((uint64_t)1 << i);
}

effects_instance *effects_set_claim(effects_set *s, effects_kind kind) {
    if (kind <= EFFECT_NONE) return NULL;

    // if it's already here, hand back the existing slot — claim is idempotent
    // per kind, the merge logic decides what to do with it.
    effects_instance *cur = effects_set_find(s, kind);
    if (cur) { set_dirty(s, (int)(cur - s->slots)); return cur; }

    for (int i = 0; i < EFFECTS_MAX_ACTIVE; i++) {
        if (!s->slots[i].active) {
            s->count++;
            set_dirty(s, i);
            return &s->slots[i];
        }
    }
    return NULL;   // full. with one slot per kind this is unreachable in practice.
}

bool effects_set_drop(effects_set *s, effects_kind kind) {
    effects_instance *e = effects_set_find(s, kind);
    if (!e) return false;
    set_dirty(s, (int)(e - s->slots));
    effects_instance_reset(e);
    if (s->count > 0) s->count--;
    return true;
}

void effects_set_clear(effects_set *s) {
    for (int i = 0; i < EFFECTS_MAX_ACTIVE; i++) {
        if (s->slots[i].active) set_dirty(s, i);
        effects_instance_reset(&s->slots[i]);
    }
    s->count = 0;
}

effects_instance *effects_set_resolve(effects_set *s, effects_handle h) {
    if (h.slot < 0 || h.slot >= EFFECTS_MAX_ACTIVE) return NULL;
    effects_instance *e = &s->slots[h.slot];
    if (!e->active || e->gen != h.gen) return NULL;
    return e;
}

effects_handle effects_set_handle_of(const effects_set *s, const effects_instance *e) {
    if (!e) return EFFECTS_HANDLE_NONE;
    ptrdiff_t idx = e - s->slots;
    if (idx < 0 || idx >= EFFECTS_MAX_ACTIVE) return EFFECTS_HANDLE_NONE;
    effects_handle h = { (int16_t)idx, e->gen };
    return h;
}

void effects_set_clear_dirty(effects_set *s) {
    s->dirty_bits = 0;
}
