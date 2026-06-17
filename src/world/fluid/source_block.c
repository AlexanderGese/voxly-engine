#include "source_block.h"
#include "fluid_level.h"
#include <string.h>
void voxl_fluid_sources_clear(voxl_fluid_source_set *s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
}

static int voxl_fluid_source_find(const voxl_fluid_source_set *s, int x, int y, int z) {
    for (int i = 0;
i < VOXL_FLUID_MAX_SOURCES;
i++) {
        const voxl_fluid_source *it = &s->items[i];
        if (it->active && it->x == x && it->y == y && it->z == z) return i;
    }
    return -1;
}

int voxl_fluid_source_add(voxl_fluid_source_set *s, int x, int y, int z, uint8_t kind) {
    if (!s) return -1;
    if (voxl_fluid_source_find(s, x, y, z) >= 0) return -1;
    for (int i = 0; i < VOXL_FLUID_MAX_SOURCES; i++) {
        if (!s->items[i].active) {
            s->items[i].x = x;
            s->items[i].y = y;
            s->items[i].z = z;
            s->items[i].kind = kind;
            s->items[i].active = 1;
            s->count++;
            return i;
        }
    }
    return -1;   // table full
}

bool voxl_fluid_source_remove(voxl_fluid_source_set *s, int x, int y, int z) {
    if (!s) return false;
int i = voxl_fluid_source_find(s, x, y, z);
if (i < 0) return false;
s->items[i].active = 0;
s->count--;
return true;
}

bool voxl_fluid_source_at(const voxl_fluid_source_set *s, int x, int y, int z) {
    if (!s) return false;
    return voxl_fluid_source_find(s, x, y, z) >= 0;
}

void voxl_fluid_sources_apply(const voxl_fluid_source_set *s, voxl_fluid_grid *g) {
    if (!s || !g) return;
for (int i = 0;
i < VOXL_FLUID_MAX_SOURCES;
int added = 0;
;
;
for (int y = 0;
y < VOXL_FLUID_GRID_N;
}
