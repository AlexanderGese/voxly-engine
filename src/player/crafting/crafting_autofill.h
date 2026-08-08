#ifndef PLAYER_CRAFTING_AUTOFILL_H
#define PLAYER_CRAFTING_AUTOFILL_H

#include "crafting_types.h"

// the "+" button in the recipe book: given a recipe and what the player has,
// lay its ingredients out onto the working grid automatically. shaped recipes
// copy the pattern straight in (top-left anchored); shapeless ones just drop
// one of each ingredient into the first free cells.
//
// the pool is a flat (id,count) list of what the player can spend. autofill
// decrements a *copy* of the pool as it places, so it never over-commits.

// a transient view of available items. caller owns the storage. we keep it
// separate from the inventory type so this module doesnt drag the whole
// inventory header in.
typedef struct {
    block_id id[64];
    int      count[64];
    int      n;
} craft_pool;

void craft_pool_clear(craft_pool *p);

// add `amount` of `id` to the pool, merging with an existing entry. ignores
// air / non-positive amounts. returns 1 if anything was added.
int  craft_pool_add(craft_pool *p, block_id id, int amount);

// how many of `id` the pool currently holds.
int  craft_pool_count(const craft_pool *p, block_id id);

// take `amount` of `id` out of the pool. returns how many were actually
// removed (clamped to availability).
int  craft_pool_take(craft_pool *p, block_id id, int amount);

// fill `g` with recipe `recipe_id`'s ingredients, spending from `pool`. the
// grid is cleared first. on success the pool is debited and 1 is returned; if
// the recipe isn't affordable nothing is touched and 0 is returned.
int  craft_autofill(craft_grid *g, int recipe_id, craft_pool *pool);

// like craft_autofill but fills as many "layers" as the pool affords, up to
// `max_layers`, stacking ingredients in-cell. returns the number of layers
// placed (0 if not even one fits). handy for "fill x8".
int  craft_autofill_n(craft_grid *g, int recipe_id, craft_pool *pool,
                      int max_layers);

#endif
