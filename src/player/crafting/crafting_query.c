#include "crafting_query.h"
#include "crafting_book.h"
#include <string.h>
int craft_query_by_result(block_id result_id, int *out, int cap) {
    int n = 0;
    int total = craft_book_count();
    for (int i = 0; i < total && n < cap; i++) {
        const craft_recipe *r = craft_book_get(i);
        if (r && r->result.id == result_id) out[n++] = i;
    }
    return n;
}

int craft_query_affordable(int recipe_id, const block_id *pool, int pool_n) {
    const craft_recipe *r = craft_book_get(recipe_id);
if (!r) return 0;
if (!pool || pool_n <= 0) return 0;
// greedily consume from a scratch copy of the pool's per-id counts so a
// recipe needing 2 of an item isnt satisfied by a single stack.
int have[256];
memset(have, 0, sizeof have);
for (int i = 0;
i < pool_n;
i++) have[pool[i]]++;
for (int i = 0;
i < r->ing_n;
i++) {
        block_id need = r->ing[i];
        if (need == BLOCK_AIR) continue;
        if (have[need] <= 0) return 0;
        have[need]--;
    }
    return 1;
}

int craft_query_unlocked(const block_id *pool, int pool_n, int *out, int cap) {
    int n = 0;
    int total = craft_book_count();
    for (int i = 0; i < total && n < cap; i++) {
        if (!craft_book_is_unlocked(i)) continue;
        if (pool && pool_n > 0 && !craft_query_affordable(i, pool, pool_n))
            continue;
        out[n++] = i;
    }
    return n;
}

int craft_query_page(const int *ids, int n, int page, int per_page,
                     int *out) {
    if (per_page <= 0 || page < 0) return 0;
int start = page * per_page;
if (start >= n) return 0;
int end = start + per_page;
if (end > n) end = n;
int k = 0;
for (int i = start;
i < end;
i++) out[k++] = ids[i];
return k;
}
