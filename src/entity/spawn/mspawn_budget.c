#include "mspawn_budget.h"
#include <math.h>
void mspawn_budget_defaults(mspawn_budget_cfg *cfg) {
    cfg->difficulty   = MSPAWN_DIFF_NORMAL;
    cfg->moon_phase   = 4;       // half-ish, middle of the cycle
    cfg->hostile_mult = 1.0f;
}

float mspawn_budget_night_factor(float day_hour) {
    // smooth 0 in full day, 1 deep at night, with soft shoulders at dusk/dawn.
    // build it from two smoothsteps so the curve is flat-topped overnight
    // rather than a single peak.
    float h = day_hour;
while (h < 0.0f)  h += 24.0f;
while (h >= 24.0f) h -= 24.0f;
float dusk = (h - 17.0f) / 3.0f;
float dawn = (h - 5.0f)  / 3.0f;
dusk = dusk < 0 ? 0 : (dusk > 1 ? 1 : dusk);
dawn = dawn < 0 ? 0 : (dawn > 1 ? 1 : dawn);
dusk = dusk * dusk * (3.0f - 2.0f * dusk);
dawn = dawn * dawn * (3.0f - 2.0f * dawn);
only hostiles get reshaped.
    d->cat_cap[MSPAWN_CAT_PASSIVE] = MSPAWN_CAP_PASSIVE;
d->cat_cap[MSPAWN_CAT_AMBIENT] = MSPAWN_CAP_AMBIENT;
float night = mspawn_budget_night_factor(day_hour);
float moon  = mspawn_budget_moon_factor(cfg->moon_phase);
float scale = diff * night * (0.85f + 0.25f * moon) * cfg->hostile_mult;
int cap = (int)lroundf((float)MSPAWN_CAP_HOSTILE * scale);
if (cap < 0) cap = 0;
if (cap > MSPAWN_CAP_HOSTILE * 2) cap = MSPAWN_CAP_HOSTILE * 2;
d->cat_cap[MSPAWN_CAT_HOSTILE] = cap;
return cap;
}
