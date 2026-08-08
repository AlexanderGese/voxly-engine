#ifndef PLAYER_CRAFTING_QUERY_H
#define PLAYER_CRAFTING_QUERY_H

#include "crafting_types.h"

// read-side helpers for the recipe-book ui: filtering, paging, and the
// "what can i make right now" reverse lookup. none of this mutates the book.

// fill `out` (caller-sized `cap`) with recipe ids that produce `result_id`.
// returns the number written. handy for the "how do i make X" panel.
int craft_query_by_result(block_id result_id, int *out, int cap);

// fill `out` with the ids of recipes the player has *unlocked*, optionally
// only those craftable from the given pool of available items. pass
// pool=NULL / pool_n=0 to skip the availability filter. returns count.
int craft_query_unlocked(const block_id *pool, int pool_n, int *out, int cap);

// can every ingredient of recipe `id` be covered by the item pool? this is
// the "is it craftable from my inventory" check the book uses to grey out
// recipes. handles duplicate ingredients correctly.
int craft_query_affordable(int recipe_id, const block_id *pool, int pool_n);

// page a list of recipe ids: copies up to `per_page` ids starting at
// page*per_page into `out`. returns count copied (0 past the end).
int craft_query_page(const int *ids, int n, int page, int per_page,
                     int *out);

#endif
