#include "enchant_roll.h"
#include "enchant_registry.h"
#include "enchant_set.h"

#include <stddef.h>

// scratch space for the weighted pass. one entry per eligible (enchant,level)
// candidate. we never have more than KINDS*MAX_LEVEL of them and in practice
// far fewer, so a fixed stack buffer is plenty.
typedef struct {
    enchant_id id;
    int        level;
    int        weight;
} roll_cand;

#define ROLL_MAX_CAND (ENCHANT_MAX_KINDS * 4)

int enchant_roll_modified_level(rng *r, int base_level, int enchantability) {
    if (base_level < 1) base_level = 1;
    if (enchantability < 1) enchantability = 1;

    // enchantability bonus: a small uniform draw scaled by the material.
    int bonus = rng_range(r, 0, enchantability / 4) +
                rng_range(r, 0, enchantability / 4) + 1;
    int lvl = base_level + bonus;

    // triangular jitter in [-0.15, +0.15] of the level, rounded. averages out
    // to the base but spreads the band so the same slot isn't deterministic.
    float t = (rng_float01(r) + rng_float01(r) - 1.0f) * 0.15f;
    float adj = (float)lvl * (1.0f + t);
    lvl = (int)(adj + 0.5f);
    if (lvl < 1) lvl = 1;
    return lvl;
}

// the level an enchant rolls at for a given modified level. each enchant has
// a per-level band [minl, maxl]; we pick the highest level whose minimum
// requirement the modified level clears. crude but it gives the satisfying
// "more power -> higher tiers" curve.
static int level_for(const enchant_def *d, int modified_level) {
    // band width: rarer enchants want more level per tier.
    int per_tier = 4 + (int)d->rarity * 3;
    int floor_lvl = 1 + (int)d->rarity * 2;
    int best = 0;
    for (int lv = 1; lv <= d->max_level; ++lv) {
        int need = floor_lvl + (lv - 1) * per_tier;
        // a generous upper band so very high power doesn't lock you out.
        int cap = need + per_tier * 2;
        if (modified_level >= need && modified_level <= cap + 64) best = lv;
    }
    return best;
}

// build the candidate list for a modified level + category. returns count and
// the summed weight via *total.
static int gather(int modified_level, enchant_cat cat,
                  roll_cand *out, int max, int *total) {
    int n = 0, sum = 0;
    int kinds = enchant_registry_count();
    for (int i = 0; i < kinds && n < max; ++i) {
        const enchant_def *d = enchant_registry_at(i);
        if (!d) continue;
        if (!enchant_applies_to(d->id, cat)) continue;
        int lv = level_for(d, modified_level);
        if (lv <= 0) continue;
        out[n].id     = d->id;
        out[n].level  = lv;
        out[n].weight = d->weight;
        sum += d->weight;
        n++;
    }
    *total = sum;
    return n;
}

// weighted draw from a candidate list. returns the index, or -1 if empty.
static int weighted_draw(rng *r, const roll_cand *c, int n, int total) {
    if (n <= 0 || total <= 0) return -1;
    int roll = rng_range(r, 0, total - 1);
    for (int i = 0; i < n; ++i) {
        roll -= c[i].weight;
        if (roll < 0) return i;
    }
    return n - 1; // rounding guard, shouldn't be reached
}

enchant_id enchant_roll_pick_one(rng *r, int modified_level,
                                 enchant_cat item_cat, int *out_level) {
    roll_cand cand[ROLL_MAX_CAND];
    int total = 0;
    int n = gather(modified_level, item_cat, cand, ROLL_MAX_CAND, &total);
    int idx = weighted_draw(r, cand, n, total);
    if (idx < 0) {
        if (out_level) *out_level = 0;
        return ENCHANT_NONE;
    }
    if (out_level) *out_level = cand[idx].level;
    return cand[idx].id;
}

int enchant_roll_slot(rng *r, int base_level, int enchantability,
                      enchant_cat item_cat, enchant_set *out) {
    enchant_set_clear(out);

    int modified = enchant_roll_modified_level(r, base_level, enchantability);

    int lvl = 0;
    enchant_id first = enchant_roll_pick_one(r, modified, item_cat, &lvl);
    if (first == ENCHANT_NONE) return 0;
    enchant_set_put(out, first, lvl);

    // extra-enchant odds, decaying as we add more. classic formula: chance is
    // (modified+1)/50, halving the modified level after each success.
    int placed = 1;
    while (placed < ENCHANT_MAX_ON_ITEM) {
        int chance = (modified + 1);  // out of 50
        if (rng_range(r, 0, 49) >= chance) break;

        int extra_lvl = 0;
        enchant_id extra = enchant_roll_pick_one(r, modified, item_cat,
                                                 &extra_lvl);
        if (extra == ENCHANT_NONE) break;

        // reject duplicates and anything mutually exclusive with the set so
        // far. one retry's worth of effort; if it clashes we just stop.
        if (enchant_set_has(out, extra)) { modified /= 2; continue; }
        if (enchant_set_conflict(out, extra) != ENCHANT_NONE) break;

        enchant_set_put(out, extra, extra_lvl);
        placed++;
        modified /= 2; // each extra is rarer than the last
    }
    return placed;
}
