#include "farming_debug.h"
#include "farming_def.h"
#include <stdio.h>

const char *farming_debug_crop_name(farming_crop_kind kind) {
    return farming_def_get(kind)->name;
}

int farming_debug_growth_pct(const farming_crop *crop) {
    const farming_def *def = farming_def_get((farming_crop_kind)crop->kind);
    int top = def->max_stage;
    if (top <= 0) return 100;

    // whole stages plus the fraction toward the next, all over the total span.
    float frac = 0.0f;
    if (def->points_per_stage > 0.0f && crop->stage < top)
        frac = crop->growth_accum / def->points_per_stage;
    if (frac > 1.0f) frac = 1.0f;

    float prog = ((float)crop->stage + frac) / (float)top;
    if (prog > 1.0f) prog = 1.0f;
    return (int)(prog * 100.0f + 0.5f);
}

int farming_debug_hydration_pct(const farming_tile *tile) {
    return (int)((tile->hydration * 100) / FARMING_HYDRATION_MAX);
}

char farming_debug_status_char(const farming_crop *crop) {
    if (crop->flags & FARMING_CROP_F_WILTED) return 'x';
    if (crop->flags & FARMING_CROP_F_MATURE) return '*';
    if (crop->stage == 0) return '.';
    return '|';
}

int farming_debug_summary(const farming_field *f, char *buf, int cap) {
    if (cap <= 0) return 0;
    // snprintf clamps for us; just hand back what it wrote.
    int n = snprintf(buf, (size_t)cap,
                     "farm: tiles=%zu crops=%zu tick=%u | produce=%d seed=%d xp=%d",
                     farming_field_tile_count(f), farming_field_crop_count(f),
                     f->tick, f->counters_produce, f->counters_seed,
                     f->counters_xp);
    if (n < 0) { buf[0] = '\0'; return 0; }
    if (n >= cap) n = cap - 1;
    return n;
}
