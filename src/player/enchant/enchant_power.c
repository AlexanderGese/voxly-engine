#include "enchant_power.h"
#include "../../world/block.h"

#include <stddef.h>

// the bookshelf ring. two square frames at dy 0 and dy 1, radius 2 on the
// xz plane, but only the 8 perimeter cells of each frame minus the corners
// we don't want doubled up. the classic layout works out to 15 valid spots.
// the air gap that has to be clear sits at radius 1 between the table and the
// shelf, on the line toward it.

// returns the table->air gap cell for a shelf at offset (dx,dz). the gap is
// the same direction but one step closer.
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
    // the 8 perimeter cells of a 5x5 ring (radius 2), per height layer.
    static const int ring[8][2] = {
        {-2, -2}, {-2, 0}, {-2, 2},
        { 0, -2},          { 0, 2},
        { 2, -2}, { 2, 0}, { 2, 2},
    };
    for (int layer = 0; layer < 2; ++layer) {
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
}

int enchant_power_count_shelves(world *w, int tx, int ty, int tz) {
    if (!w) return 0;

    int dx[ENCHANT_MAX_BOOKSHELVES];
    int dy[ENCHANT_MAX_BOOKSHELVES];
    int dz[ENCHANT_MAX_BOOKSHELVES];
    int n = enchant_power_shelf_offsets(dx, dy, dz, ENCHANT_MAX_BOOKSHELVES);

    int count = 0;
    for (int i = 0; i < n; ++i) {
        block_id shelf = world_get_block(w, tx + dx[i], ty + dy[i], tz + dz[i]);
        if (shelf != BLOCK_WOOD) continue; // wood doubles as a bookshelf here

        // the gap between table and shelf must not be opaque, else the line
        // of sight is blocked and the shelf doesn't contribute.
        int gx, gz;
        gap_cell(dx[i], dz[i], &gx, &gz);
        block_id gap = world_get_block(w, tx + gx, ty + dy[i], tz + gz);
        if (block_is_opaque(gap)) continue;

        if (++count >= ENCHANT_MAX_BOOKSHELVES) break;
    }
    return count;
}

void enchant_power_slot_levels(int shelves, int out_levels[ENCHANT_TABLE_SLOTS]) {
    if (shelves < 0) shelves = 0;
    if (shelves > ENCHANT_MAX_BOOKSHELVES) shelves = ENCHANT_MAX_BOOKSHELVES;

    // base is a small jitter-free seed; the bottom slot tracks raw power,
    // the middle splits the difference, the top is a fraction. this mirrors
    // the familiar 1..30 spread without the per-open randomness (that lives
    // in the roll picker, which is the right place for it).
    int top    = shelves / 3 + 1;
    int middle = (shelves * 2) / 3 + 1;
    int bottom = shelves + 1;

    // never offer a slot below 1 or above 30.
    out_levels[0] = top    < 1 ? 1 : top;
    out_levels[1] = middle < 1 ? 1 : middle;
    out_levels[2] = bottom > 30 ? 30 : bottom;
}
