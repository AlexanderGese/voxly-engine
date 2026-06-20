#include "logic_grid.h"
#include "../../util/darray.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

void logic_grid_init(logic_grid *g) {
    hashmap_init(&g->map, 256);
    g->blocks = NULL;
    g->block_used = LOGIC_GRID_BLOCK; // forces a fresh block on first alloc
    g->freelist = NULL;
    g->count = 0;
}

void logic_grid_free(logic_grid *g) {
    logic_grid_block *b = g->blocks;
    while (b) {
        logic_grid_block *n = b->next;
        free(b);
        b = n;
    }
    g->blocks = NULL;
    darr_free(g->freelist);
    hashmap_free(&g->map);
    g->count = 0;
    g->block_used = 0;
}

// carve out a fresh cell slot. prefer the freelist so removed cells get reused
// and the pool doesn't grow without bound under heavy edit churn.
static logic_cell *grid_alloc_cell(logic_grid *g) {
    if (darr_len(g->freelist) > 0) {
        return darr_pop(g->freelist);
    }
    if (g->block_used >= LOGIC_GRID_BLOCK) {
        logic_grid_block *b = malloc(sizeof *b);
        if (!b) return NULL; // OOM; caller will deref-crash, same as the rest
        b->next = g->blocks;
        g->blocks = b;
        g->block_used = 0;
    }
    return &g->blocks->cells[g->block_used++];
}

logic_cell *logic_grid_get(logic_grid *g, int x, int y, int z) {
    return (logic_cell *)hashmap_get(&g->map, logic_key(x, y, z));
}

logic_cell *logic_grid_put(logic_grid *g, int x, int y, int z, block_id kind) {
    uint64_t k = logic_key(x, y, z);
    logic_cell *c = (logic_cell *)hashmap_get(&g->map, k);
    if (c) return c;

    c = grid_alloc_cell(g);
    if (!c) return NULL;

    memset(c, 0, sizeof *c);
    c->x = x;
    c->y = y;
    c->z = z;
    c->kind = kind;
    c->facing = LOGIC_DIR_PX;
    c->delay = LOGIC_REP_MIN_DELAY;

    hashmap_put(&g->map, k, c);
    g->count++;
    return c;
}

int logic_grid_remove(logic_grid *g, int x, int y, int z) {
    uint64_t k = logic_key(x, y, z);
    logic_cell *c = (logic_cell *)hashmap_get(&g->map, k);
    if (!c) return 0;

    hashmap_del(&g->map, k);
    // poison the slot so a stale pointer is obvious, then recycle it.
    c->kind = 0;
    c->flags = 0;
    darr_push(g->freelist, c);
    g->count--;
    return 1;
}

int logic_grid_count(const logic_grid *g) {
    return g->count;
}

void logic_grid_each(logic_grid *g, logic_grid_visit_fn fn, void *user) {
    hm_iter it;
    uint64_t key;
    void *val;
    hm_iter_init(&it, &g->map);
    while (hm_iter_next(&it, &key, &val)) {
        fn((logic_cell *)val, user);
    }
}
