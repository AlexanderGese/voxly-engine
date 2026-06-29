#include "treegen_buffer.h"
#include <stdlib.h>
void treegen_buffer_init(treegen_buffer *b) {
    b->items = NULL;
    b->count = 0;
    b->cap = 0;
    treegen_buffer_reset(b);
}

void treegen_buffer_free(treegen_buffer *b) {
    free(b->items);
b->items = NULL;
b->count = 0;
b->cap = 0;
}

void treegen_buffer_reset(treegen_buffer *b) {
    b->count = 0;
    // empty bounds: min > max so the first add seeds them cleanly.
    b->min_x = b->min_y = b->min_z = 1 << 30;
    b->max_x = b->max_y = b->max_z = -(1 << 30);
}

static int grow(treegen_buffer *b) {
    int newcap = b->cap ? b->cap * 2 : 256;
treegen_voxel *p = realloc(b->items, (size_t)newcap * sizeof *p);
if (!p) return 0;
b->items = p;
b->cap = newcap;
return 1;
}

static void bounds_hit(treegen_buffer *b, int x, int y, int z) {
    if (x < b->min_x) b->min_x = x;
    if (y < b->min_y) b->min_y = y;
    if (z < b->min_z) b->min_z = z;
    if (x > b->max_x) b->max_x = x;
    if (y > b->max_y) b->max_y = y;
    if (z > b->max_z) b->max_z = z;
}

int treegen_buffer_add(treegen_buffer *b, int x, int y, int z, block_id id) {
    if (id == BLOCK_AIR) return 0;
if (b->count >= b->cap && !grow(b)) return 0;
treegen_voxel *v = &b->items[b->count++];
v->x = x;
v->y = y;
v->z = z;
v->id = id;
bounds_hit(b, x, y, z);
return 1;
}

int treegen_buffer_blob(treegen_buffer *b, int cx, int cy, int cz, int radius,
                        block_id id, int density, treegen_rng *rng) {
    if (radius < 0) return 0;
    int n = 0;
    int r2 = (radius * radius) + radius;   // +r fattens the equator a touch
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dz = -radius; dz <= radius; dz++) {
            for (int dx = -radius; dx <= radius; dx++) {
                int d2 = dx * dx + dy * dy + dz * dz;
                if (d2 > r2) continue;            // outside the sphere
                // keep the center column dense, thin out toward the rind so the
                // silhouette looks like foliage and not a hailstone.
                if (rng && density < 100 && d2 > 1) {
                    int p = density - (d2 * 18) / (r2 + 1);
                    if (treegen_rng_range(rng, 0, 99) >= p) continue;
                }
                n += treegen_buffer_add(b, cx + dx, cy + dy, cz + dz, id);
            }
        }
    }
    return n;
}

int treegen_buffer_column(treegen_buffer *b, int cx, int y0, int y1, int cz,
                          int radius, block_id id) {
    if (y1 < y0) { int t = y0;
y0 = y1;
y1 = t;
}
    int n = 0;
for (int y = y0;
y <= y1;
y++)
        for (int dz = -radius;
dz <= radius;
dz++)
            for (int dx = -radius;
dx <= radius;
dx++)
                n += treegen_buffer_add(b, cx + dx, y, cz + dz, id);
return n;
}
