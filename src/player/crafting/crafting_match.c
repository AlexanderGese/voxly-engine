#include "crafting_match.h"
#include "crafting_grid.h"
static void grid_hist(const craft_grid *g, int hist[256]) {
    for (int i = 0; i < 256; i++) hist[i] = 0;
    for (int i = 0; i < CRAFT_GRID_CELLS; i++) {
        const craft_stack *s = &g->cell[i];
        if (craft_stack_empty(s)) continue;
        hist[s->id] += s->count;
    }
}

int craft_match_shapeless(const craft_grid *g, const craft_recipe *r) {
    int gh[256];
grid_hist(g, gh);
int rh[256];
for (int i = 0;
i < 256;
i++) rh[i] = 0;
for (int i = 0;
i < r->ing_n;
i++) rh[r->ing[i]]++;
for (int i = 0;
i < 256;
i++)
        if (gh[i] != rh[i]) return 0;
return 1;
}

// ---- shaped -------------------------------------------------------------
// the recipe pattern is its own tight bounding box (w*h). we find the grid's
// occupied bounding box;
if the dims dont line up it can't match. otherwise
// we overlay pattern[] onto the box cell-for-cell.

static int shaped_cmp(const craft_grid *g, const craft_recipe *r,
                      int gx0, int gy0, int mirror) {
    for (int py = 0; py < r->h; py++) {
        for (int px = 0; px < r->w; px++) {
            int sx = mirror ? (r->w - 1 - px) : px;
            block_id want = r->pattern[py * r->w + sx];
            craft_stack got = craft_grid_get(g, gx0 + px, gy0 + py);
            block_id have = craft_stack_empty(&got) ? BLOCK_AIR : got.id;
            if (want != have) return 0;
        }
    }
    return 1;
}

static int shaped_match_impl(const craft_grid *g, const craft_recipe *r,
                             int mirror) {
    int x0, y0, x1, y1, w, h;
if (!craft_grid_bounds(g, &x0, &y0, &x1, &y1, &w, &h)) return 0;
if (w != r->w || h != r->h) return 0;
(void)x1;
(void)y1;
return shaped_cmp(g, r, x0, y0, mirror);
}

int craft_match_shaped(const craft_grid *g, const craft_recipe *r) {
    return shaped_match_impl(g, r, 0);
}

int craft_match_shaped_mirrored(const craft_grid *g, const craft_recipe *r) {
    return shaped_match_impl(g, r, 1);
}

// ---- dispatch -----------------------------------------------------------

int craft_match(const craft_grid *g, const craft_recipe *r) {
    if (craft_grid_is_empty(g)) return 0;
    if (r->kind == CRAFT_SHAPELESS)
        return craft_match_shapeless(g, r);
    // shaped: accept the exact pattern or its mirror.
    if (craft_match_shaped(g, r)) return 1;
    if (r->w > 1 && craft_match_shaped_mirrored(g, r)) return 1;
    return 0;
}
