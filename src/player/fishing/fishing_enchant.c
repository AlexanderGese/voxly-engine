#include "fishing_enchant.h"
#include <stddef.h>

// look up a single enchant's level on a set. 0 if absent. the set is kept
// sorted by id elsewhere, but it's tiny, so a linear scan is fine and saves us
// depending on that invariant.
static int level_of(const enchant_set *set, enchant_id id) {
    if (!set) return 0;
    for (int i = 0; i < set->count; i++) {
        if (set->entry[i].id == id)
            return set->entry[i].level;
    }
    return 0;
}

static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

int fishing_enchant_lure_level(const enchant_set *set) {
    // efficiency doubles as lure. the enchant caps at level 10 but our rod only
    // wants 0..3, so scale it down rather than clamp hard — every ~3 levels of
    // efficiency buys a lure tier.
    int eff = level_of(set, ENCHANT_EFFICIENCY);
    return clampi((eff + 2) / 3, 0, FISHING_LURE_MAX);
}

int fishing_enchant_luck_level(const enchant_set *set) {
    // fortune stands in for luck of the sea, mapped one-to-one but clamped.
    int fortune = level_of(set, ENCHANT_FORTUNE);
    return clampi(fortune, 0, FISHING_LUCK_MAX);
}

fishing_rod fishing_enchant_resolve(const enchant_set *set) {
    int lure = fishing_enchant_lure_level(set);
    int luck = fishing_enchant_luck_level(set);
    fishing_rod r = fishing_rod_make(lure, luck);

    // unbreaking on the rod nudges the line strength up a hair — a sturdier rod
    // tolerates a bit more tension before the line gives.
    int unbreaking = level_of(set, ENCHANT_UNBREAKING);
    r.max_tension += (float)unbreaking * 4.0f;
    return r;
}

int fishing_enchant_applies(enchant_id id) {
    // only the ids we actually read above (plus unbreaking) make sense on a rod.
    switch (id) {
    case ENCHANT_EFFICIENCY:
    case ENCHANT_FORTUNE:
    case ENCHANT_UNBREAKING:
        return 1;
    default:
        return 0;
    }
}
