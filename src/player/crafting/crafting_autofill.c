#include "crafting_autofill.h"
#include "crafting_grid.h"
#include "crafting_book.h"
#include <string.h>

void craft_pool_clear(craft_pool *p) { p->n = 0; }

static int pool_find(const craft_pool *p, block_id id) {
    for (int i = 0; i < p->n; i++)
        if (p->id[i] == id) return i;
    return -1;
}

int craft_pool_add(craft_pool *p, block_id id, int amount) {
    if (id == BLOCK_AIR || amount <= 0) return 0;
    int i = pool_find(p, id);
    if (i >= 0) { p->count[i] += amount; return 1; }
    if (p->n >= 64) return 0;   // pool is full, drop it. 64 distinct is plenty.
    p->id[p->n] = id;
    p->count[p->n] = amount;
    p->n++;
    return 1;
}

int craft_pool_count(const craft_pool *p, block_id id) {
    int i = pool_find(p, id);
    return i >= 0 ? p->count[i] : 0;
}

int craft_pool_take(craft_pool *p, block_id id, int amount) {
    int i = pool_find(p, id);
    if (i < 0 || amount <= 0) return 0;
    int took = p->count[i] < amount ? p->count[i] : amount;
    p->count[i] -= took;
    return took;
}

// can we cover every ingredient of `r` against `pool`, `layers` times over?
// builds a small demand histogram so duplicate ingredients are summed.
static int affordable_layers(const craft_recipe *r, const craft_pool *pool,
                             int layers) {
    int need[256];
    memset(need, 0, sizeof need);
    if (r->kind == CRAFT_SHAPED) {
        for (int i = 0; i < r->w * r->h; i++)
            if (r->pattern[i] != BLOCK_AIR) need[r->pattern[i]] += layers;
    } else {
        for (int i = 0; i < r->ing_n; i++) need[r->ing[i]] += layers;
    }
    for (int id = 0; id < 256; id++)
        if (need[id] && craft_pool_count(pool, (block_id)id) < need[id])
            return 0;
    return 1;
}

// drop one of each ingredient into the grid, anchored top-left for shaped and
// row-packed for shapeless. spends from the pool. assumes affordability was
// already checked.
static void place_layer(craft_grid *g, const craft_recipe *r,
                        craft_pool *pool) {
    if (r->kind == CRAFT_SHAPED) {
        for (int y = 0; y < r->h; y++) {
            for (int x = 0; x < r->w; x++) {
                block_id id = r->pattern[y * r->w + x];
                if (id == BLOCK_AIR) continue;
                craft_grid_place_one(g, x, y, id);
                craft_pool_take(pool, id, 1);
            }
        }
    } else {
        // shapeless: walk cells in row-major order, dropping one item each.
        int k = 0;
        for (int c = 0; c < CRAFT_GRID_CELLS && k < r->ing_n; c++) {
            int x = c % CRAFT_GRID_MAX, y = c / CRAFT_GRID_MAX;
            block_id id = r->ing[k];
            if (craft_grid_place_one(g, x, y, id)) {
                craft_pool_take(pool, id, 1);
                k++;
            }
        }
    }
}

int craft_autofill(craft_grid *g, int recipe_id, craft_pool *pool) {
    return craft_autofill_n(g, recipe_id, pool, 1) > 0 ? 1 : 0;
}

int craft_autofill_n(craft_grid *g, int recipe_id, craft_pool *pool,
                     int max_layers) {
    const craft_recipe *r = craft_book_get(recipe_id);
    if (!r || max_layers < 1) return 0;
    if (!affordable_layers(r, pool, 1)) return 0;

    craft_grid_clear(g);
    int placed = 0;
    for (int l = 0; l < max_layers; l++) {
        // each extra layer needs another full set still in the pool.
        if (!affordable_layers(r, pool, 1)) break;
        place_layer(g, r, pool);
        placed++;
    }
    return placed;
}
