#include "effects_curative.h"
#include "effects_def.h"

#include <stddef.h>

static uint64_t kbit(effects_kind kind) {
    if (kind <= EFFECT_NONE || kind >= EFFECT_KIND_COUNT) return 0;
    return (uint64_t)1 << (int)kind;
}

void effects_immunity_clear(effects_immunity *im) { im->bits = 0; }

void effects_immunity_add(effects_immunity *im, effects_kind kind) {
    im->bits |= kbit(kind);
}

void effects_immunity_remove(effects_immunity *im, effects_kind kind) {
    im->bits &= ~kbit(kind);
}

bool effects_immunity_has(const effects_immunity *im, effects_kind kind) {
    return (im->bits & kbit(kind)) != 0;
}

effects_immunity effects_immunity_undead(void) {
    // skeletons/zombies: poison and regen do nothing. (the "healing hurts them"
    // rule lives over in combat where the damage actually resolves — here we
    // just stop the green particles from ever showing up.)
    effects_immunity im = { 0 };
    effects_immunity_add(&im, EFFECT_POISON);
    effects_immunity_add(&im, EFFECT_REGENERATION);
    return im;
}

int effects_curative_milk(effects_set *s) {
    int removed = 0;
    for (int i = 0; i < EFFECTS_MAX_ACTIVE; i++) {
        effects_instance *e = &s->slots[i];
        if (!e->active) continue;
        const effects_def *d = effects_def_get(e->kind);
        if (!d->curable) continue;   // night vision etc ride it out
        if (effects_set_drop(s, e->kind)) removed++;
    }
    return removed;
}

int effects_curative_antidote(effects_set *s) {
    int removed = 0;
    for (int i = 0; i < EFFECTS_MAX_ACTIVE; i++) {
        effects_instance *e = &s->slots[i];
        if (!e->active) continue;
        const effects_def *d = effects_def_get(e->kind);
        if (d->category != EFFECT_CAT_HARMFUL) continue;
        if (!d->curable) continue;
        if (effects_set_drop(s, e->kind)) removed++;
    }
    return removed;
}

bool effects_curative_remove(effects_set *s, effects_kind kind) {
    return effects_set_drop(s, kind);
}

bool effects_curative_admits(const effects_immunity *im, effects_kind kind) {
    if (kind <= EFFECT_NONE || kind >= EFFECT_KIND_COUNT) return false;
    if (im && effects_immunity_has(im, kind)) return false;
    return true;
}
