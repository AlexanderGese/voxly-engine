#include "nav_query_box.h"

#include <math.h>
#include <stdlib.h>

static int blk(float v) { return (int)floorf(v); }

// is this cell's floor inside the box? xz uses the block the floor sits in;
// y compares the floor coord against the box's vertical span (inclusive-ish).
static int cell_in_box(const nav_cell *c, aabb box) {
    int lo_x = blk(box.min.x), hi_x = blk(box.max.x);
    int lo_z = blk(box.min.z), hi_z = blk(box.max.z);
    int lo_y = blk(box.min.y), hi_y = blk(box.max.y);
    if (c->x < lo_x || c->x > hi_x) return 0;
    if (c->z < lo_z || c->z > hi_z) return 0;
    if (c->y < lo_y || c->y > hi_y) return 0;
    return 1;
}

int nav_query_box(const nav_grid *g, aabb box, int *out, int max) {
    int n = 0;
    for (int i = 0; i < g->count && n < max; i++) {
        if (cell_in_box(&g->cells[i], box)) out[n++] = i;
    }
    return n;
}

int nav_query_box_count(const nav_grid *g, aabb box) {
    int n = 0;
    for (int i = 0; i < g->count; i++)
        if (cell_in_box(&g->cells[i], box)) n++;
    return n;
}

// tiny xorshift32 so this file is self-contained; the engine rng is overkill
// for picking a spawn tile and i didn't want the include.
static uint32_t xs32(uint32_t *s) {
    uint32_t x = *s ? *s : 0x9e3779b9u;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *s = x;
    return x;
}

int nav_query_box_random(const nav_grid *g, aabb box, uint32_t *rng_state) {
    // reservoir sample: one pass, no allocation, uniform over the matches.
    int chosen = -1;
    int seen = 0;
    for (int i = 0; i < g->count; i++) {
        if (!cell_in_box(&g->cells[i], box)) continue;
        seen++;
        // replace with probability 1/seen -> uniform at the end.
        if ((xs32(rng_state) % (uint32_t)seen) == 0) chosen = i;
    }
    return chosen;
}

int nav_query_nearest_in_region(const nav_grid *g, vec3 p, int region, int r) {
    int cx = blk(p.x), cz = blk(p.z), fy = blk(p.y);

    // expanding rings, same shape as nav_query_nearest, but we only accept a
    // hit whose region matches and we keep the closest-by-floor-height match in
    // each ring so we don't grab a cell way above/below the asker.
    int best = -1, best_dy = 0x7fffffff;
    for (int ring = 0; ring <= r; ring++) {
        for (int dz = -ring; dz <= ring; dz++) {
            for (int dx = -ring; dx <= ring; dx++) {
                if (ring && abs(dx) != ring && abs(dz) != ring) continue;
                // probe a small vertical band around the asker's feet.
                for (int dy = 0; dy <= NAV_STEP_DOWN + 1; dy++) {
                    int idx = nav_grid_find(g,
                        nav_coord_make(cx + dx, fy - 1 - dy, cz + dz));
                    if (idx < 0) continue;
                    if (g->cells[idx].region != (uint16_t)region) continue;
                    if (dy < best_dy) { best = idx; best_dy = dy; }
                }
            }
        }
        if (best >= 0) return best;   // first ring with a match wins
    }
    return best;
}
