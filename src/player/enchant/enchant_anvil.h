#ifndef PLAYER_ENCHANT_ANVIL_H
#define PLAYER_ENCHANT_ANVIL_H

#include "enchant_types.h"

// the anvil: combine a target item with a sacrifice (another item or a book)
// and merge their enchants. matching enchants level up (or bump by one if
// already equal-max), the rest get spliced in if compatible. each item also
// carries a "prior work" counter that doubles the repair cost every time,
// which is the game's way of stopping you from infinitely anvil-spamming.

// everything the anvil needs to know about one input item.
typedef struct {
    enchant_set  ench;
    enchant_cat  cat;
    int          prior_work;   // number of past anvil uses, 0 for fresh
} enchant_anvil_item;

// the computed outcome of a combine. valid==0 means the inputs are
// incompatible (e.g. wrong category) and nothing should happen.
typedef struct {
    int          valid;
    enchant_set  result;
    int          xp_cost;       // total levels the player must pay
    int          new_prior_work;// prior-work counter to stamp on the output
    int          too_expensive; // 1 if cost exceeded the hard cap (>= 40)
} enchant_anvil_result;

// the hard cap the source material uses: anything 40+ levels is "too
// expensive" and refused in survival.
#define ENCHANT_ANVIL_MAX_COST 40

// compute the merge of `target` with `sacrifice`. does not mutate inputs.
void enchant_anvil_combine(const enchant_anvil_item *target,
                           const enchant_anvil_item *sacrifice,
                           enchant_anvil_result *out);

// the prior-work penalty for a given counter value: 2^n - 1 xp levels.
int  enchant_anvil_prior_penalty(int prior_work);

#endif
