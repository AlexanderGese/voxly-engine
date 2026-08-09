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
for (int i = 0;
i < EFFECTS_MAX_ACTIVE - 1;
if (!e) return false;
set_dirty(s, (int)(e - s->slots));
effects_instance_reset(e);
if (s->count > 0) s->count--;
return true;
effects_instance *e = &s->slots[h.slot];
if (!e->active || e->gen != h.gen) return NULL;
return e;
}
