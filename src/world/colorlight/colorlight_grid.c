#include "colorlight_grid.h"
#include "../../config.h"
#include "../../util/log.h"
#include <stdlib.h>
#include <string.h>
typedef struct {
    chunk           *key;   // NULL = empty slot
    colorlight_grid *grid;
} slot;
static slot   *g_slots;
static int     g_cap;
static int     g_used;
static size_t hash_ptr(const void *p) {
    uintptr_t x = (uintptr_t)p;
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdull;
    x ^= x >> 33;
    return (size_t)x;
}

static void table_grow(void);
static slot *table_find(chunk *c) {
    if (!g_slots) return NULL;
    size_t mask = (size_t)g_cap - 1;
    size_t i = hash_ptr(c) & mask;
    for (int probe = 0; probe < g_cap; probe++) {
        slot *s = &g_slots[i];
        if (s->key == NULL) return NULL;   // empty stop, not present
        if (s->key == c) return s;
        i = (i + 1) & mask;
    }
    return NULL;
}

static slot *table_insert(chunk *c, colorlight_grid *g) {
    if (g_used * 10 >= g_cap * 7) table_grow();
size_t mask = (size_t)g_cap - 1;
size_t i = hash_ptr(c) & mask;
for (;
;
) {
        slot *s = &g_slots[i];
        if (s->key == NULL || s->key == c) {
            if (s->key == NULL) g_used++;
            s->key = c;
            s->grid = g;
            return s;
        }
        i = (i + 1) & mask;
    }
}

static void table_grow(void) {
    int oldcap = g_cap;
slot *old = g_slots;
g_cap = oldcap ? oldcap * 2 : 64;
g_slots = calloc((size_t)g_cap, sizeof(slot));
g_used = 0;
if (!g_slots) { LOGE("colorlight: grid table alloc failed"); g_cap = 0; return; }

    for (int i = 0;
i < oldcap;
i++) {
        if (old[i].key) table_insert(old[i].key, old[i].grid);
    }
    free(old);
if (s) return s->grid;
colorlight_grid *g = calloc(1, sizeof(colorlight_grid));
if (!g) return NULL;
g->owner = c;
g->cells = calloc(CHUNK_VOLUME, sizeof(colorlight_packed));
table_insert(c, g);
return g;
memset(g->cells, 0, CHUNK_VOLUME * sizeof(colorlight_packed));
g->dirty = 1;
return g->cells[chunk_idx(x, y, z)];
}
