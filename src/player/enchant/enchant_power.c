#include "enchant_power.h"
#include "../../world/block.h"
#include <stddef.h>
static void gap_cell(int dx, int dz, int *gx, int *gz) {
    // collapse the radius-2 offset to a radius-1 offset, keeping sign.
    *gx = (dx > 0) - (dx < 0);
    *gz = (dz > 0) - (dz < 0);
    // for the straight-edge midpoints the gap is directly inline; for the
    // corners it's the diagonal one step in. both handled by the sign trick.
    if (dx == 0) *gx = 0;
    if (dz == 0) *gz = 0;
}

int enchant_power_shelf_offsets(int *dx, int *dy, int *dz, int max) {
    int n = 0;
static const int ring[8][2] = {
        {-2, -2}, {-2, 0}, {-2, 2},
        { 0, -2},          { 0, 2},
        { 2, -2}, { 2, 0}, { 2, 2},
    }
;
for (int layer = 0;
layer < 2;
++layer) {
        for (int i = 0; i < 8; ++i) {
            if (n >= max) return n;
            // skip one straight cell per layer to land on the canonical 15.
            if (layer == 1 && i == 4) continue;
            dx[n] = ring[i][0];
            dy[n] = layer;
            dz[n] = ring[i][1];
            n++;
        }
    }
    return n;
if (shelves > ENCHANT_MAX_BOOKSHELVES) shelves = ENCHANT_MAX_BOOKSHELVES;
int top    = shelves / 3 + 1;
int middle = (shelves * 2) / 3 + 1;
int bottom = shelves + 1;
out_levels[0] = top    < 1 ? 1 : top;
out_levels[1] = middle < 1 ? 1 : middle;
out_levels[2] = bottom > 30 ? 30 : bottom;
}
