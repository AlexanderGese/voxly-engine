#include "colorlight_grid.h"
#include "../../config.h"
#include "../../util/log.h"

#include <stdlib.h>
#include <string.h>

// side table mapping chunk* -> grid. open addressed, linear probe. grows when
// it gets above ~70% full. keys are pointers so we hash with a fibonacci mix.
// this never holds more than a couple hundred chunks; if it ever did i'd swap
// in util/hashmap, but that wants string keys and i don't feel like adapting it.

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
    for (;;) {
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

    for (int i = 0; i < oldcap; i++) {
        if (old[i].key) table_insert(old[i].key, old[i].grid);
    }
    free(old);
}

colorlight_grid *colorlight_grid_peek(chunk *c) {
    slot *s = table_find(c);
    return s ? s->grid : NULL;
}

colorlight_grid *colorlight_grid_for(chunk *c) {
    slot *s = table_find(c);
    if (s) return s->grid;

    colorlight_grid *g = calloc(1, sizeof(colorlight_grid));
    if (!g) return NULL;
    g->owner = c;
    g->cells = calloc(CHUNK_VOLUME, sizeof(colorlight_packed));
    if (!g->cells) { free(g); return NULL; }
    g->dirty = 1;

    table_insert(c, g);
    return g;
}

void colorlight_grid_release(chunk *c) {
    slot *s = table_find(c);
    if (!s) return;
    free(s->grid->cells);
    free(s->grid);
    // tombstone-free deletion: clear and reinsert the rest of the probe run so
    // lookups don't break on the now-empty hole.
    size_t mask = (size_t)g_cap - 1;
    size_t i = (size_t)(s - g_slots);
    s->key = NULL; s->grid = NULL; g_used--;
    size_t j = i;
    for (;;) {
        j = (j + 1) & mask;
        slot *t = &g_slots[j];
        if (t->key == NULL) break;
        chunk *k = t->key; colorlight_grid *gg = t->grid;
        t->key = NULL; t->grid = NULL; g_used--;
        table_insert(k, gg);
    }
}

void colorlight_grid_clear(colorlight_grid *g) {
    if (!g) return;
    memset(g->cells, 0, CHUNK_VOLUME * sizeof(colorlight_packed));
    g->dirty = 1;
}

static int in_bounds(int x, int y, int z) {
    return x >= 0 && x < CHUNK_SIZE_X
        && z >= 0 && z < CHUNK_SIZE_Z
        && y >= 0 && y < CHUNK_SIZE_Y;
}

colorlight_packed colorlight_grid_get(const colorlight_grid *g, int x, int y, int z) {
    if (!g || !in_bounds(x, y, z)) return 0;
    return g->cells[chunk_idx(x, y, z)];
}

void colorlight_grid_set(colorlight_grid *g, int x, int y, int z, colorlight_packed p) {
    if (!g || !in_bounds(x, y, z)) return;
    int i = chunk_idx(x, y, z);
    if (g->cells[i] == p) return;
    g->cells[i] = p;
    g->dirty = 1;
}

uint8_t colorlight_grid_get_chan(const colorlight_grid *g, int x, int y, int z, int chan) {
    return colorlight_packed_chan(colorlight_grid_get(g, x, y, z), chan);
}

void colorlight_grid_set_chan(colorlight_grid *g, int x, int y, int z, int chan, uint8_t v) {
    if (!g || !in_bounds(x, y, z)) return;
    int i = chunk_idx(x, y, z);
    colorlight_packed np = colorlight_packed_set_chan(g->cells[i], chan, v);
    if (np == g->cells[i]) return;
    g->cells[i] = np;
    g->dirty = 1;
}

int colorlight_grid_live_count(void) {
    return g_used;
}
