#include "fishing_rod.h"

static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

fishing_rod fishing_rod_default(void) {
    fishing_rod r;
    r.lure        = 0;
    r.luck        = 0;
    r.max_tension = 100.0f;
    r.cast_power  = 9.0f;
    return r;
}

fishing_rod fishing_rod_make(int lure, int luck) {
    fishing_rod r = fishing_rod_default();
    r.lure = clampi(lure, 0, FISHING_LURE_MAX);
    r.luck = clampi(luck, 0, FISHING_LUCK_MAX);
    // a more enchanted rod tends to be a better-built one, so let luck buy a
    // little extra line strength. small, but it matters on the heavy catches.
    r.max_tension += (float)r.luck * 8.0f;
    return r;
}

float fishing_rod_lure_bonus(const fishing_rod *r) {
    return (float)r->lure * 5.0f;
}

float fishing_rod_treasure_bias(const fishing_rod *r) {
    // each luck level adds 35% to treasure weight. luck 3 -> ~2.05x.
    return 1.0f + (float)r->luck * 0.35f;
}

float fishing_rod_junk_bias(const fishing_rod *r) {
    // and trims junk by 25% a level, never below a tenth so junk never vanishes.
    float b = 1.0f - (float)r->luck * 0.25f;
    return b < 0.1f ? 0.1f : b;
}
