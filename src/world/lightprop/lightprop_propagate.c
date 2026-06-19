#include "lightprop_propagate.h"
#include "lightprop_access.h"
#include "lightprop_step.h"

int lp_seed(world *w, lp_channel ch, lp_queue *q,
            int wx, int wy, int wz, uint8_t level) {
    if (level == 0) return 0;
    if (!lp_y_in_range(wy)) return 0;
    if (!lp_cell_loaded(w, wx, wz)) return 0;
    // a source sitting inside an opaque block can't spread (e.g. a torch that
    // got buried). still write the value so removal can find it, but don't flood.
    block_id here = lp_get_block(w, wx, wy, wz);
    if (lp_get_light(w, ch, wx, wy, wz) < level)
        lp_set_light(w, ch, wx, wy, wz, level);
    if (!lp_transmits(here) && level < MAX_LIGHT) return 0;
    return lp_queue_push(q, wx, wy, wz, level);
}

void lp_flood(world *w, lp_channel ch, lp_queue *q) {
    lp_node n;
    while (lp_queue_pop(q, &n)) {
        // re-read: a later push may have raised this cell past what we recorded.
        uint8_t cur = lp_get_light(w, ch, n.x, n.y, n.z);
        if (cur > n.level) continue;     // someone brighter already passed here
        if (cur <= 1) continue;          // nothing left to give

        for (int d = 0; d < 6; d++) {
            int nx = n.x + LP_DX[d];
            int ny = n.y + LP_DY[d];
            int nz = n.z + LP_DZ[d];
            if (!lp_y_in_range(ny)) continue;
            if (!lp_cell_loaded(w, nx, nz)) continue;

            block_id nid = lp_get_block(w, nx, ny, nz);
            uint8_t give = lp_step_propagate(ch, d, cur, nid);
            if (give == 0) continue;

            if (lp_get_light(w, ch, nx, ny, nz) >= give) continue;
            lp_set_light(w, ch, nx, ny, nz, give);
            lp_queue_push(q, nx, ny, nz, give);
        }
    }
}

// same flood, but clamped to a box around (cx,cy,cz) of half-extent `radius`.
// used when we only care about repairing the immediate neighbourhood of an edit
// and don't want a single bright torch dragging the bfs across half the world.
// light that would have spilled past the box is simply not propagated; the seam
// stitcher picks it up later if it matters.
void lp_flood_bounded(world *w, lp_channel ch, lp_queue *q,
                      int cx, int cy, int cz, int radius) {
    lp_node n;
    while (lp_queue_pop(q, &n)) {
        uint8_t cur = lp_get_light(w, ch, n.x, n.y, n.z);
        if (cur > n.level) continue;
        if (cur <= 1) continue;

        for (int d = 0; d < 6; d++) {
            int nx = n.x + LP_DX[d];
            int ny = n.y + LP_DY[d];
            int nz = n.z + LP_DZ[d];

            // chebyshev box clamp. cheap, and matches how chunks tile.
            if (nx < cx - radius || nx > cx + radius) continue;
            if (nz < cz - radius || nz > cz + radius) continue;
            if (ny < cy - radius || ny > cy + radius) continue;
            if (!lp_y_in_range(ny)) continue;
            if (!lp_cell_loaded(w, nx, nz)) continue;

            block_id nid = lp_get_block(w, nx, ny, nz);
            uint8_t give = lp_step_propagate(ch, d, cur, nid);
            if (give == 0) continue;

            if (lp_get_light(w, ch, nx, ny, nz) >= give) continue;
            lp_set_light(w, ch, nx, ny, nz, give);
            lp_queue_push(q, nx, ny, nz, give);
        }
    }
}
