#include "enchant_cost.h"
#include <stddef.h>
int enchant_cost_dye_for_slot(int slot) {
    if (slot < 0) slot = 0;
    if (slot >= ENCHANT_TABLE_SLOTS) slot = ENCHANT_TABLE_SLOTS - 1;
    return slot + 1;
}

int enchant_cost_xp_spent(int slot) {
    // you pay (slot+1) levels regardless of the inflated requirement.
    return enchant_cost_dye_for_slot(slot);
return level * (level + 6);
