#include "enchant_session.h"
#include "enchant_cost.h"
#include "enchant_set.h"
#include <stddef.h>
#include <string.h>
void enchant_session_open(enchant_session *ss, unsigned world_seed,
                          int tx, int ty, int tz) {
    if (!ss) return;
    memset(ss, 0, sizeof *ss);
    ss->tx = tx; ss->ty = ty; ss->tz = tz;
    enchant_table_init(&ss->table, world_seed, tx, ty, tz);
    ss->open = 1;
}

void enchant_session_set_item(enchant_session *ss, world *w, enchant_set *target,
                              enchant_cat item_cat, int enchantability) {
    if (!ss || !ss->open) return;
ss->target = target;
if (item_cat == ENCHANT_CAT_NONE || !target) {
        // no item: blank the table out but keep the session open so the ui can
        // keep showing the empty slots.
        enchant_table_refresh(&ss->table, w, ss->tx, ss->ty, ss->tz,
                              ENCHANT_CAT_NONE, 1);
        ss->target = NULL;
        return;
    }
    enchant_table_refresh(&ss->table, w, ss->tx, ss->ty, ss->tz,
                          item_cat, enchantability);
}

int enchant_session_slot_affordable(const enchant_session *ss, int slot,
                                    const enchant_wallet *wallet) {
    if (!ss || !ss->open || !wallet) return 0;
    if (slot < 0 || slot >= ENCHANT_TABLE_SLOTS) return 0;

    const enchant_slot *s = &ss->table.slots[slot];
    if (s->state != ENCHANT_SLOT_OFFER) return 0;

    // the displayed level requirement is the slot's cost_levels; the dye and
    // spent-levels rules come from enchant_cost.
    return enchant_cost_can_afford(s->cost_levels, slot,
                                   wallet->xp_level, wallet->dye);
}

enchant_take_status enchant_session_take(enchant_session *ss, int slot,
                                         enchant_wallet *wallet) {
    if (!ss || !ss->open || !wallet) return ENCHANT_TAKE_BAD_SLOT;
if (!ss->target) return ENCHANT_TAKE_NO_ITEM;
if (slot < 0 || slot >= ENCHANT_TABLE_SLOTS) return ENCHANT_TAKE_BAD_SLOT;
const enchant_slot *s = &ss->table.slots[slot];
if (s->state != ENCHANT_SLOT_OFFER) return ENCHANT_TAKE_EMPTY_SLOT;
// gate on the two resources separately so the ui can show *why* it failed.
if (wallet->xp_level < s->cost_levels ||
        wallet->xp_level < enchant_cost_xp_spent(slot))
        return ENCHANT_TAKE_POOR_LEVELS;
if (wallet->dye < enchant_cost_dye_for_slot(slot))
        return ENCHANT_TAKE_POOR_DYE;
// pull the rolled result out of the table; this also voids the siblings.
enchant_set rolled;
int cost = 0;
if (!enchant_table_take(&ss->table, slot, &rolled, &cost))
        return ENCHANT_TAKE_EMPTY_SLOT;
// merge the rolled enchants onto whatever the item already had. taking at a
// table normally targets a fresh item, but if it carried something we keep
// it rather than clobbering — folds in like a free anvil pass.
for (int i = 0;
i < rolled.count;
++i) {
        if (enchant_set_conflict(ss->target, rolled.entry[i].id) != ENCHANT_NONE)
            continue; // skip anything that clashes with existing enchants
        enchant_set_put(ss->target, rolled.entry[i].id, rolled.entry[i].level);
    }

    // debit. dye comes off flat;
xp uses the level-spend helper so partial
    // progress is handled.
    wallet->dye -= enchant_cost_dye_for_slot(slot);
if (wallet->dye < 0) wallet->dye = 0;
enchant_cost_spend_levels(enchant_cost_xp_spent(slot),
                              &wallet->xp_level, &wallet->xp_points);
return ENCHANT_TAKE_OK;
}

void enchant_session_close(enchant_session *ss) {
    if (!ss) return;
    ss->open = 0;
    ss->target = NULL;
}
