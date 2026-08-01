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

    // rising edge across dusk (17 -> 20), falling edge across dawn (5 -> 8).
    float dusk = (h - 17.0f) / 3.0f;
    float dawn = (h - 5.0f)  / 3.0f;
    dusk = dusk < 0 ? 0 : (dusk > 1 ? 1 : dusk);
    dawn = dawn < 0 ? 0 : (dawn > 1 ? 1 : dawn);
    // smoothstep both
    dusk = dusk * dusk * (3.0f - 2.0f * dusk);
    dawn = dawn * dawn * (3.0f - 2.0f * dawn);

    if (h >= 12.0f) {
        // afternoon/evening half: ramping up via dusk.
        return dusk;
    } else {
        // morning half: full at midnight, ramping down via dawn.
        return 1.0f - dawn;
    }
}

float mspawn_budget_moon_factor(int moon_phase) {
    // phase 0 full, phase 4 new. cosine over the 8-step cycle gives a smooth
    // bright->dark->bright, then remap so full moon = 1, new moon = ~0.15 so
    // mobs never fully dry up on a clear night.
    int p = moon_phase & 7;
    float c = (cosf((float)p / 8.0f * 6.2831853f) + 1.0f) * 0.5f; // 1 at p=0
    return 0.15f + 0.85f * c;
}

int mspawn_budget_apply(mspawn_density *d, const mspawn_budget_cfg *cfg,
                        float day_hour) {
    // passive/ambient stay at baseline; only hostiles get reshaped.
    d->cat_cap[MSPAWN_CAT_PASSIVE] = MSPAWN_CAP_PASSIVE;
    d->cat_cap[MSPAWN_CAT_AMBIENT] = MSPAWN_CAP_AMBIENT;

    if (cfg->difficulty == MSPAWN_DIFF_PEACEFUL) {
        d->cat_cap[MSPAWN_CAT_HOSTILE] = 0;     // no monsters, full stop
        return 0;
    }

    float diff = (cfg->difficulty == MSPAWN_DIFF_HARD) ? 1.35f : 1.0f;
    float night = mspawn_budget_night_factor(day_hour);
    float moon  = mspawn_budget_moon_factor(cfg->moon_phase);

    // moon only sweetens the deal a little (up to +25%), night is the big gate.
    float scale = diff * night * (0.85f + 0.25f * moon) * cfg->hostile_mult;
    int cap = (int)lroundf((float)MSPAWN_CAP_HOSTILE * scale);
    if (cap < 0) cap = 0;
    if (cap > MSPAWN_CAP_HOSTILE * 2) cap = MSPAWN_CAP_HOSTILE * 2;

    d->cat_cap[MSPAWN_CAT_HOSTILE] = cap;
    return cap;
}
