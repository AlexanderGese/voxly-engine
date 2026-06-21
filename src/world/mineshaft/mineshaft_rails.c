#include "mineshaft_rails.h"
#include "../block_ext.h"

// a "straight" cell links exactly two opposite neighbours: N+S or E+W. corners
// (N+E etc) and tees we treat as non-straight so rails don't kink awkwardly.
int mineshaft_rails_is_straight(const mineshaft_grid *g, int cx, int cz) {
    if (!mineshaft_grid_in_bounds(g, cx, cz)) return 0;
    uint8_t l = g->cells[cz * g->w + cx].links;
    int ns = ((l & MS_LINK_N) != 0) + ((l & MS_LINK_S) != 0);
    int ew = ((l & MS_LINK_E) != 0) + ((l & MS_LINK_W) != 0);
    if (ns == 2 && ew == 0) return 1;   // vertical run
    if (ew == 2 && ns == 0) return 1;   // horizontal run
    return 0;
}

int mineshaft_rails_minecart(mineshaft_buffer *b, const mineshaft_config *cfg,
                             int x, int floor_y, int z, mineshaft_rng *rng) {
    // low odds; the caller already decided this is a plausible spot.
    if (!mineshaft_rng_chance(rng, 0.18f)) return 0;
    // the cart body sits one block above the rail. no cart block id, so it's a
    // little wooden hull stand-in (planks) - reads as "something got left here".
    int n = 0;
    n += mineshaft_buffer_add(b, x, floor_y + 1, z, BLOCK_PLANKS);
    // a stray gold nugget's worth of loot spilled beside it, sometimes.
    if (mineshaft_rng_chance(rng, 0.3f))
        n += mineshaft_buffer_add(b, x, floor_y + 1, z + 1, cfg->ore_rare);
    return n;
}

int mineshaft_rails_cell(mineshaft_buffer *b, const mineshaft_config *cfg,
                         const mineshaft_grid *g, int cx, int cz,
                         mineshaft_box cell_box, int floor_y, float wear,
                         mineshaft_rng *rng) {
    if (!mineshaft_rails_is_straight(g, cx, cz)) return 0;

    const mineshaft_cell *c = &g->cells[cz * g->w + cx];
    int ew = (c->links & (MS_LINK_E | MS_LINK_W)) != 0;
    int mx = (cell_box.x0 + cell_box.x1) / 2;
    int mz = (cell_box.z0 + cell_box.z1) / 2;
    int ry = floor_y + 1;   // rails rest on the floor, occupy the air above it

    int n = 0;
    int cart_dropped = 0;
    if (ew) {
        for (int x = cell_box.x0; x < cell_box.x1; x++) {
            // rusted-out gaps: a per-block roll against wear leaves holes.
            if (mineshaft_rng_f01(rng) < wear) continue;
            n += mineshaft_buffer_add(b, x, ry, mz, cfg->mat_rail);
            // rare derelict cart on an intact stretch.
            if (!cart_dropped) {
                int got = mineshaft_rails_minecart(b, cfg, x, floor_y, mz, rng);
                if (got) { n += got; cart_dropped = 1; }
            }
        }
    } else {
        for (int z = cell_box.z0; z < cell_box.z1; z++) {
            if (mineshaft_rng_f01(rng) < wear) continue;
            n += mineshaft_buffer_add(b, mx, ry, z, cfg->mat_rail);
            if (!cart_dropped) {
                int got = mineshaft_rails_minecart(b, cfg, mx, floor_y, z, rng);
                if (got) { n += got; cart_dropped = 1; }
            }
        }
    }
    return n;
}
