#include "effects_hud.h"
#include "effects_def.h"

#include <stddef.h>
#include <stdio.h>

static const char *k_roman[] = {
    "", "I", "II", "III", "IV", "V", "VI", "VII", "VIII"
};

const char *effects_hud_level_label(int level) {
    if (level >= 0 && level < (int)(sizeof k_roman / sizeof k_roman[0]))
        return k_roman[level];
    return "?";   // past VIII the ui just prints the raw number elsewhere
}

void effects_hud_format_time(int seconds_left, char *buf, int cap) {
    if (cap <= 0) return;
    if (seconds_left < 0) {
        snprintf(buf, cap, "%s", "∞");   // infinity glyph
        return;
    }
    int m = seconds_left / 60;
    int s = seconds_left % 60;
    snprintf(buf, cap, "%d:%02d", m, s);
}

// fill one entry from a live instance.
static void fill_entry(effects_hud_entry *out, const effects_instance *e, float tps) {
    const effects_def *d = effects_def_get(e->kind);
    out->kind = e->kind;
    out->name = d->name;
    out->level = e->amplifier + 1;
    out->fraction = effects_instance_fraction(e);
    out->beneficial = (d->category == EFFECT_CAT_BENEFICIAL);
    out->tint = vec3_new(d->r / 255.0f, d->g / 255.0f, d->b / 255.0f);

    if (e->duration == EFFECTS_DURATION_INFINITE) {
        out->seconds_left = -1;
        out->fading = false;
    } else {
        float secs = (tps > 0.0f) ? (float)e->duration / tps : (float)e->duration;
        out->seconds_left = (int)(secs + 0.5f);
        out->fading = out->seconds_left < 5;
    }
}

// ordering: buffs before debuffs (beneficial sorts first), then soonest expiry
// first within a group. infinite effects sink to the bottom of their group.
static int entry_less(const effects_hud_entry *a, const effects_hud_entry *b) {
    if (a->beneficial != b->beneficial) return a->beneficial ? 1 : 0;
    int as = a->seconds_left, bs = b->seconds_left;
    if (as < 0) as = 0x7fffffff;   // infinite => effectively last
    if (bs < 0) bs = 0x7fffffff;
    if (as != bs) return as < bs;
    // tiebreak on kind id for a stable, deterministic layout.
    return (int)a->kind < (int)b->kind;
}

int effects_hud_build(effects_hud_list *list, const effects_set *s, float tps) {
    list->count = 0;
    if (!s) return 0;

    for (int i = 0; i < EFFECTS_MAX_ACTIVE; i++) {
        const effects_instance *e = &s->slots[i];
        if (!e->active) continue;
        if (effects_def_get(e->kind)->hidden) continue;
        if (list->count >= EFFECTS_HUD_MAX) break;
        fill_entry(&list->entries[list->count++], e, tps);
    }

    // small list, insertion sort is plenty and keeps it stable.
    for (int i = 1; i < list->count; i++) {
        effects_hud_entry key = list->entries[i];
        int j = i - 1;
        while (j >= 0 && entry_less(&key, &list->entries[j])) {
            list->entries[j + 1] = list->entries[j];
            j--;
        }
        list->entries[j + 1] = key;
    }

    return list->count;
}
