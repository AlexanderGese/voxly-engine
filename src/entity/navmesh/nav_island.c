#include "nav_island.h"

#include <string.h>

// find (or append) the summary slot for a region id. linear scan — region
// counts per patch are small enough that a map would be overkill, and the
// flood hands them out densely so the first-seen order is already tight.
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

    for (int i = 0; i < g->count; i++) {
        const nav_cell *c = &g->cells[i];
        if (c->region == NAV_REGION_NONE) continue;   // unlabelled, skip

        nav_island *it = slot_for(set, c->region);
        if (!it) continue;     // ran out of buckets; rest just don't get summed

        if (it->cells == 0) it->floor_y = c->y;
        it->cells++;
        it->sx += c->x;
        it->sz += c->z;
        if (c->x < it->min_x) it->min_x = c->x;
        if (c->z < it->min_z) it->min_z = c->z;
        if (c->x > it->max_x) it->max_x = c->x;
        if (c->z > it->max_z) it->max_z = c->z;
    }
    return set->count;
}

const nav_island *nav_island_get(const nav_island_set *set, int region) {
    for (int i = 0; i < set->count; i++)
        if (set->items[i].region == (uint16_t)region) return &set->items[i];
    return NULL;
}

vec3 nav_island_centroid(const nav_island *isl) {
    if (!isl || isl->cells == 0) return VEC3_ZERO;
    float cx = (float)isl->sx / (float)isl->cells + 0.5f;
    float cz = (float)isl->sz / (float)isl->cells + 0.5f;
    // stand on top of the representative floor, same convention as the cells.
    return vec3_new(cx, (float)(isl->floor_y + 1), cz);
}

int nav_island_largest(const nav_island_set *set) {
    int best = -1, best_cells = -1;
    for (int i = 0; i < set->count; i++) {
        if (set->items[i].cells > best_cells) {
            best_cells = set->items[i].cells;
            best = i;
        }
    }
    if (best < 0) return NAV_REGION_NONE;
    return set->items[best].region;
}
