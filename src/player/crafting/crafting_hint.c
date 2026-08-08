#include "crafting_hint.h"
#include "crafting_grid.h"
#include "crafting_book.h"
#include "crafting_match.h"
#include <string.h>
static void grid_hist(const craft_grid *g, int h[256]) {
    memset(h, 0, sizeof(int) * 256);
    for (int i = 0; i < CRAFT_GRID_CELLS; i++) {
        const craft_stack *s = &g->cell[i];
        if (craft_stack_empty(s)) continue;
        h[s->id] += s->count;
    }
}

static void recipe_hist(const craft_recipe *r, int h[256]) {
    memset(h, 0, sizeof(int) * 256);
if (r->kind == CRAFT_SHAPED) {
        for (int i = 0; i < r->w * r->h; i++)
            if (r->pattern[i] != BLOCK_AIR) h[r->pattern[i]]++;
    } else {
        for (int i = 0;
i < r->ing_n;
i++) h[r->ing[i]]++;
}
}

// returns total distance;
splits the count into missing (need to add) and
// extra (need to remove) via the out params if non-NULL.
static int hist_distance(const int gh[256], const int rh[256],
                         int *missing, int *extra) {
    int miss = 0, ext = 0;
    for (int i = 0; i < 256; i++) {
        int d = rh[i] - gh[i];
        if (d > 0) miss += d;        // recipe wants more of i
        else if (d < 0) ext += -d;   // grid has surplus of i
    }
    if (missing) *missing = miss;
    if (extra) *extra = ext;
    return miss + ext;
}

int craft_hint_distance(const craft_grid *g, const craft_recipe *r) {
    int gh[256], rh[256];
grid_hist(g, gh);
recipe_hist(r, rh);
return hist_distance(gh, rh, NULL, NULL);
int total = craft_book_count();
int gh[256];
grid_hist(g, gh);
int n = 0;
for (int i = 0;
i < total;
grid_hist(g, gh);
recipe_hist(r, rh);
int worst_id = BLOCK_AIR, worst = 0;
for (int i = 0;
i < 256;
return worst;
}
