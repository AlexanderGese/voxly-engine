#include "lightprop_remove.h"
#include "lightprop_access.h"
#include "lightprop_step.h"

void lp_seed_removal(world *w, lp_channel ch, lp_queue *rq,
                     int wx, int wy, int wz, uint8_t old_level) {
    if (old_level == 0) return;
    if (!lp_y_in_range(wy)) return;
    if (!lp_cell_loaded(w, wx, wz)) return;
    lp_set_light(w, ch, wx, wy, wz, 0);          // darken now, repair later
    lp_queue_push(rq, wx, wy, wz, old_level);
}

void lp_unflood(world *w, lp_channel ch, lp_queue *rq, lp_queue *aq) {
    lp_node n;
    while (lp_queue_pop(rq, &n)) {
        uint8_t old = n.level;   // the brightness this cell USED to radiate

        for (int d = 0; d < 6; d++) {
            int nx = n.x + LP_DX[d];
            int ny = n.y + LP_DY[d];
            int nz = n.z + LP_DZ[d];
            if (!lp_y_in_range(ny)) continue;
            if (!lp_cell_loaded(w, nx, nz)) continue;

            uint8_t nlevel = lp_get_light(w, ch, nx, ny, nz);
            if (nlevel == 0) continue;

            // would our old light have reached this neighbour at exactly its
            // current value? if so it was OUR child -> tear it down too. the sky
            // drop makes a vertical column inherit the full level, so check that
            // path as well or we'd leave a bright streak hanging.
            block_id nid = lp_get_block(w, nx, ny, nz);
            uint8_t expected = lp_step_propagate(ch, d, old, nid);

            if (nlevel != 0 && nlevel == expected) {
                // lit by us. remove it and recurse, carrying ITS old level.
                lp_set_light(w, ch, nx, ny, nz, 0);
                lp_queue_push(rq, nx, ny, nz, nlevel);
            } else if (nlevel >= old) {
                // brighter than the front we're erasing -> independent source.
                // hand it to the re-light pass to fill the hole back in.
                lp_queue_push(aq, nx, ny, nz, nlevel);
            }
        }
    }
}

// seed a whole vertical sky column for removal in one go. when a block is placed
// that blocks a shaft, every transparent cell BELOW the new block that was on the
// free MAX_LIGHT drop is now orphaned. walking them one bfs node at a time works
// but seeding the column directly is much cheaper and avoids the queue churning
// through 60+ stacked cells. cells that weren't actually on the column (already
// dimmer, i.e. lit sideways) are skipped and left for lp_unflood to sort out.
void lp_seed_sky_column_removal(world *w, lp_queue *rq, int wx, int wy, int wz) {
    for (int y = wy; y >= 0; y--) {
        block_id id = lp_get_block(w, wx, y, wz);
        if (!lp_transmits(id)) break;             // hit the floor of the shaft
        uint8_t cur = lp_get_light(w, LP_SKY, wx, y, wz);
        if (cur != MAX_LIGHT) break;              // no longer the free column
        lp_set_light(w, LP_SKY, wx, y, wz, 0);
        lp_queue_push(rq, wx, y, wz, MAX_LIGHT);
        // a tinted-but-transparent cell (water) below would have re-attenuated,
        // so the pure column ends here even if it still transmits.
        if (lp_attenuation(id) != 0) break;
    }
}
