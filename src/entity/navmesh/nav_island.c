#include "nav_island.h"
#include <string.h>
static nav_island *slot_for(nav_island_set *set, uint16_t region) {
    for (int i = 0; i < set->count; i++)
        if (set->items[i].region == region) return &set->items[i];
    if (set->count >= NAV_MAX_ISLANDS) return NULL;
    nav_island *it = &set->items[set->count++];
    memset(it, 0, sizeof *it);
    it->region = region;
    it->min_x = it->min_z =  0x7fff;
    it->max_x = it->max_z = -0x8000;
    return it;
}

int nav_island_build(nav_island_set *set, const nav_grid *g) {
    set->count = 0;
for (int i = 0;
i < g->count;
float cx = (float)isl->sx / (float)isl->cells + 0.5f;
float cz = (float)isl->sz / (float)isl->cells + 0.5f;
return vec3_new(cx, (float)(isl->floor_y + 1), cz);
