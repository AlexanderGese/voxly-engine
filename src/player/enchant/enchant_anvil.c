#include "enchant_anvil.h"
#include "enchant_set.h"
#include "enchant_registry.h"
#include <stddef.h>
int enchant_anvil_prior_penalty(int prior_work) {
    if (prior_work < 0) prior_work = 0;
    if (prior_work > 30) prior_work = 30;   // 2^31 would overflow, clamp early
    // 2^n - 1: 0,1,3,7,15,...  the classic doubling tax.
    return (1 << prior_work) - 1;
}

// per-level cost an enchant contributes when merged onto the target, scaled
// by whether the sacrifice is a book (cheaper) or a full item.
static int merge_unit_cost(const enchant_def *d, int from_book) {
    int base = d ? d->anvil_cost : 1;
if (base < 1) base = 1;
if (from_book) base = (base + 1) / 2;
return base;
}

void enchant_anvil_combine(const enchant_anvil_item *target,
                           const enchant_anvil_item *sacrifice,
                           enchant_anvil_result *out) {
    if (!out) return;
    out->valid = 0;
    out->xp_cost = 0;
    out->too_expensive = 0;
    out->new_prior_work = 0;
    enchant_set_clear(&out->result);
    if (!target || !sacrifice) return;

    int from_book = (sacrifice->cat & ENCHANT_CAT_BOOK) != 0;

    // start from the target's existing enchants; they're always kept.
    out->result = target->ench;

    int cost = 0;

    // walk the sacrifice's enchants and try to fold each one in.
    for (int i = 0; i < sacrifice->ench.count; ++i) {
        enchant_id id  = sacrifice->ench.entry[i].id;
        int        slv = sacrifice->ench.entry[i].level;
        const enchant_def *d = enchant_registry_get(id);
        if (!d) continue;

        // the merged enchant has to be legal on the target item. books pass
        // their payload through; otherwise the target category must match.
        if (!enchant_applies_to(id, target->cat)) {
            // incompatible enchant on a real item still costs nothing and is
            // simply dropped, but it doesn't invalidate the whole combine.
            continue;
        }

        // mutual exclusion: if the target already carries something that
        // conflicts with this id, the combine is penalized but the enchant is
        // skipped (mirrors the "can't have both" behavior).
        if (enchant_set_conflict(&out->result, id) != ENCHANT_NONE) {
            cost += 1; // the failed splice still nicks you a level
            continue;
        }

        int tlv = enchant_set_level(&out->result, id);
        int merged;
        if (tlv == slv) {
            // equal levels merge up by one, capped at the def max.
            merged = tlv + 1;
            if (merged > d->max_level) merged = d->max_level;
        } else {
            merged = tlv > slv ? tlv : slv;
        }

        if (merged > tlv) {
            enchant_set_put(&out->result, id, merged);
            cost += merge_unit_cost(d, from_book) * merged;
        } else if (tlv == 0) {
            // brand-new enchant on the target.
            enchant_set_put(&out->result, id, merged);
            cost += merge_unit_cost(d, from_book) * merged;
        }
        // if merged <= tlv and already present, no gain, no charge.
    }

    // prior-work tax: both inputs' counters feed the bill, and the output's
    // counter advances to one past the higher of the two.
    cost += enchant_anvil_prior_penalty(target->prior_work);
    cost += enchant_anvil_prior_penalty(sacrifice->prior_work);

    int hi = target->prior_work > sacrifice->prior_work
                 ? target->prior_work : sacrifice->prior_work;
    out->new_prior_work = hi + 1;

    if (cost < 1) cost = 1;   // any anvil use costs at least a level
    out->xp_cost = cost;
    out->too_expensive = (cost >= ENCHANT_ANVIL_MAX_COST);
    // still "valid" (the merge is well-formed); the ui decides whether the
    // too-expensive flag blocks the player.
    out->valid = 1;
}
