#include "crafting_grid.h"

static inline int idx(int x, int y) { return y * CRAFT_GRID_MAX + x; }

static inline int in_bounds(int x, int y) {
    return x >= 0 && x < CRAFT_GRID_MAX && y >= 0 && y < CRAFT_GRID_MAX;
}

void craft_grid_clear(craft_grid *g) {
    for (int i = 0; i < CRAFT_GRID_CELLS; i++)
        g->cell[i] = craft_stack_make(BLOCK_AIR, 0);
}

int craft_grid_set(craft_grid *g, int x, int y, craft_stack s) {
    if (!in_bounds(x, y)) return 0;
    g->cell[idx(x, y)] = s;
    return 1;
}

craft_stack craft_grid_get(const craft_grid *g, int x, int y) {
    if (!in_bounds(x, y)) return craft_stack_make(BLOCK_AIR, 0);
    return g->cell[idx(x, y)];
}

int craft_grid_place_one(craft_grid *g, int x, int y, block_id id) {
    if (!in_bounds(x, y) || id == BLOCK_AIR) return 0;
    craft_stack *c = &g->cell[idx(x, y)];
    if (craft_stack_empty(c)) {
        *c = craft_stack_make(id, 1);
        return 1;
    }
    if (c->id == id) {
        // grid cells dont have a stack ceiling really, but keep it sane.
        if (c->count >= CRAFT_MAX_RESULT) return 0;
        c->count++;
        return 1;
    }
    // occupied by a different item, refuse.
    return 0;
}

block_id craft_grid_take_one(craft_grid *g, int x, int y) {
    if (!in_bounds(x, y)) return BLOCK_AIR;
    craft_stack *c = &g->cell[idx(x, y)];
    if (craft_stack_empty(c)) return BLOCK_AIR;
    block_id id = c->id;
    if (--c->count <= 0) *c = craft_stack_make(BLOCK_AIR, 0);
    return id;
}

int craft_grid_occupied(const craft_grid *g) {
    int n = 0;
    for (int i = 0; i < CRAFT_GRID_CELLS; i++)
        if (!craft_stack_empty(&g->cell[i])) n++;
    return n;
}

int craft_grid_bounds(const craft_grid *g, int *x0, int *y0,
                      int *x1, int *y1, int *w, int *h) {
    int minx = CRAFT_GRID_MAX, miny = CRAFT_GRID_MAX, maxx = -1, maxy = -1;
    for (int y = 0; y < CRAFT_GRID_MAX; y++) {
        for (int x = 0; x < CRAFT_GRID_MAX; x++) {
            if (craft_stack_empty(&g->cell[idx(x, y)])) continue;
            if (x < minx) minx = x;
            if (y < miny) miny = y;
            if (x > maxx) maxx = x;
            if (y > maxy) maxy = y;
        }
    }
    if (maxx < 0) return 0;  // empty
    if (x0) *x0 = minx;
    if (y0) *y0 = miny;
    if (x1) *x1 = maxx;
    if (y1) *y1 = maxy;
    if (w)  *w  = maxx - minx + 1;
    if (h)  *h  = maxy - miny + 1;
    return 1;
}

int craft_grid_is_empty(const craft_grid *g) {
    return craft_grid_occupied(g) == 0;
}
