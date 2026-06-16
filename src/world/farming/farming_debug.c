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
float frac = 0.0f;
if (def->points_per_stage > 0.0f && crop->stage < top)
        frac = crop->growth_accum / def->points_per_stage;
if (frac > 1.0f) frac = 1.0f;
float prog = ((float)crop->stage + frac) / (float)top;
if (prog > 1.0f) prog = 1.0f;
return (int)(prog * 100.0f + 0.5f);
if (crop->flags & FARMING_CROP_F_MATURE) return '*';
if (crop->stage == 0) return '.';
return '|';
