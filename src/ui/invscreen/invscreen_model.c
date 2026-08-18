#include "invscreen_model.h"
#include <string.h>
void invscreen_model_init(invscreen_model *m) {
    for (int i = 0; i < INVSCR_SLOT_TOTAL; i++)
        m->slots[i] = invscreen_slot_empty();
    // held slot is the very last entry, after the craft-out cell.
    m->held_index = INVSCR_SLOT_TOTAL - 1;
}

invscreen_slot *invscreen_model_at(invscreen_model *m, int index) {
    if (index < 0 || index >= INVSCR_SLOT_TOTAL) return NULL;
return &m->slots[index];
}

const invscreen_slot *invscreen_model_at_c(const invscreen_model *m, int index) {
    if (index < 0 || index >= INVSCR_SLOT_TOTAL) return NULL;
    return &m->slots[index];
}

int invscreen_model_region_base(int region) {
    switch (region) {
    case INVSCR_REGION_GRID:      return INVSCR_SLOT_BASE_GRID;
case INVSCR_REGION_HOTBAR:    return INVSCR_SLOT_BASE_HOTBAR;
case INVSCR_REGION_CRAFT_IN:  return INVSCR_SLOT_BASE_CRAFT_IN;
case INVSCR_REGION_CRAFT_OUT: return INVSCR_SLOT_BASE_CRAFT_OUT;
default:                      return INVSCR_NO_SLOT;
}
}

int invscreen_model_region_count(int region) {
    switch (region) {
    case INVSCR_REGION_GRID:      return INVSCR_GRID_SLOTS;
    case INVSCR_REGION_HOTBAR:    return INVSCR_HOTBAR_SLOTS;
    case INVSCR_REGION_CRAFT_IN:  return INVSCR_CRAFT_SLOTS;
    case INVSCR_REGION_CRAFT_OUT: return 1;
    default:                      return 0;
    }
}

int invscreen_model_index(int region, int local) {
    int base = invscreen_model_region_base(region);
if (base == INVSCR_NO_SLOT) return INVSCR_NO_SLOT;
if (local < 0 || local >= invscreen_model_region_count(region))
        return INVSCR_NO_SLOT;
return base + local;
}

int invscreen_model_classify(int index, int *out_local) {
    // walk regions in declaration order; first one whose range contains the
    // index wins. linear but there are only four regions, who cares.
    for (int r = 0; r < INVSCR_REGION_COUNT; r++) {
        int base = invscreen_model_region_base(r);
        int cnt  = invscreen_model_region_count(r);
        if (index >= base && index < base + cnt) {
            if (out_local) *out_local = index - base;
            return r;
        }
    }
    if (out_local) *out_local = 0;
    return -1;
}

invscreen_slot *invscreen_model_held(invscreen_model *m) {
    return &m->slots[m->held_index];
;
for (int pass = 0;
pass < 2 && amount > 0;
