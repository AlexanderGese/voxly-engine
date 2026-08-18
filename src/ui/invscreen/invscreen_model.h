#ifndef UI_INVSCREEN_MODEL_H
#define UI_INVSCREEN_MODEL_H

// the backing store for the screen. one flat array of slots indexed by the
// region bases in invscreen_config.h, plus the cursor-held slot tacked on the
// end. the layout/draw/drag code only ever touches slots through here so we can
// keep the index math in one place and not scatter region arithmetic around.

#include "invscreen_slot.h"

typedef struct {
    // [0 .. INVSCR_SLOT_TOTAL). last entry (CRAFT_OUT_BASE+... actually the very
    // last) is the held/cursor slot. see invscreen_config region bases.
    invscreen_slot slots[INVSCR_SLOT_TOTAL];

    // index of the cursor-held slot. constant, but named so call sites read well.
    int held_index;
} invscreen_model;

void invscreen_model_init(invscreen_model *m);

// raw access by absolute slot index. bounds-checked; out of range -> NULL.
invscreen_slot *invscreen_model_at(invscreen_model *m, int index);
const invscreen_slot *invscreen_model_at_c(const invscreen_model *m, int index);

// region helpers. `local` is 0-based within the region. returns absolute index
// or INVSCR_NO_SLOT if local is out of that region's range.
int invscreen_model_region_base(int region);
int invscreen_model_region_count(int region);
int invscreen_model_index(int region, int local);

// classify an absolute index back into (region, local). returns region or -1.
int invscreen_model_classify(int index, int *out_local);

// the held cursor slot. convenience wrappers around the tail entry.
invscreen_slot *invscreen_model_held(invscreen_model *m);
int invscreen_model_holding(const invscreen_model *m);

// try to drop `id` x `amount` into the backpack grid then hotbar, stacking
// first. returns the leftover that didn't fit. mirrors container_add's two-pass
// shape so behavior matches chests.
int invscreen_model_pickup(invscreen_model *m, block_id id, int amount);

// total count of a given id across grid + hotbar. used by craft matching so it
// can tell whether the player can afford a recipe.
int invscreen_model_total(const invscreen_model *m, block_id id);

#endif
