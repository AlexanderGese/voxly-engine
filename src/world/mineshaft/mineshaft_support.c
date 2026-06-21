#include "mineshaft_support.h"
#define SUPPORT_PITCH   3
int mineshaft_support_frame(mineshaft_buffer *b, const mineshaft_config *cfg,
                            int x0, int z0, int x1, int z1,
                            int floor_y, int ceil_y, float decay,
                            mineshaft_rng *rng) {
    int n = 0;
    int beam_y = ceil_y;          // beam sits at the ceiling line

    // post A
    if (!mineshaft_rng_chance(rng, decay))
        n += mineshaft_buffer_post(b, x0, floor_y, beam_y, z0, cfg->mat_support);
    // post B
    if (!mineshaft_rng_chance(rng, decay))
        n += mineshaft_buffer_post(b, x1, floor_y, beam_y, z1, cfg->mat_support);

    // the beam spans the posts at beam_y. a rotted beam looks sadder than no
    // beam, so it gets its own (lower) decay roll.
    if (!mineshaft_rng_chance(rng, decay * 0.5f)) {
        if (x0 == x1) {
            int lo = z0 < z1 ? z0 : z1, hi = z0 < z1 ? z1 : z0;
            for (int z = lo; z <= hi; z++)
                n += mineshaft_buffer_add(b, x0, beam_y, z, cfg->mat_beam);
        } else {
            int lo = x0 < x1 ? x0 : x1, hi = x0 < x1 ? x1 : x0;
            for (int x = lo; x <= hi; x++)
                n += mineshaft_buffer_add(b, x, beam_y, z0, cfg->mat_beam);
        }
    }
    return n;
}

int mineshaft_support_cell(mineshaft_buffer *b, const mineshaft_config *cfg,
                           const mineshaft_grid *g, int cx, int cz,
                           mineshaft_box cell_box,
                           int floor_y, int ceil_y, mineshaft_rng *rng) {
    // dominant passage axis from the cell links. e/w passage -> posts straddle
    // it on the z walls, and vice versa.
    const mineshaft_cell *c = &g->cells[cz * g->w + cx];
int ew = (c->links & (MS_LINK_E | MS_LINK_W)) != 0;
int x0 = cell_box.x0 + 1, x1 = cell_box.x1 - 2;
int z0 = cell_box.z0 + 1, z1 = cell_box.z1 - 2;
if (x1 < x0) x1 = x0;
if (z1 < z0) z1 = z0;
int n = 0;
if (ew) {
        for (int x = x0; x <= x1; x += SUPPORT_PITCH) {
            float decay = mineshaft_rng_f01(rng) * 0.4f;   // up to 40% rot
            n += mineshaft_support_frame(b, cfg, x, z0, x, z1,
                                         floor_y, ceil_y, decay, rng);
        }
    } else {
        for (int z = z0;
z <= z1;
z += SUPPORT_PITCH) {
            float decay = mineshaft_rng_f01(rng) * 0.4f;
            n += mineshaft_support_frame(b, cfg, x0, z, x1, z,
                                         floor_y, ceil_y, decay, rng);
        }
    }
    // note: floor rails are handled separately by mineshaft_rails so straight
    // runs stay continuous across cell borders. supports just do the timbers.
    return n;
}

int mineshaft_support_junction(mineshaft_buffer *b, const mineshaft_config *cfg,
                               mineshaft_box cell_box,
                               int floor_y, int ceil_y, mineshaft_rng *rng) {
    int cx = (cell_box.x0 + cell_box.x1) / 2;
    int cz = (cell_box.z0 + cell_box.z1) / 2;
    int n = 0;

    // a stout center post almost always survives - it's what kept the roof up.
    if (!mineshaft_rng_chance(rng, 0.15f))
        n += mineshaft_buffer_post(b, cx, floor_y, ceil_y, cz, cfg->mat_support);

    // crossing beams both axes at the ceiling line.
    for (int x = cell_box.x0 + 1; x < cell_box.x1 - 1; x++)
        n += mineshaft_buffer_add(b, x, ceil_y, cz, cfg->mat_beam);
    for (int z = cell_box.z0 + 1; z < cell_box.z1 - 1; z++)
        n += mineshaft_buffer_add(b, cx, ceil_y, z, cfg->mat_beam);

    // corner posts, half gone to time.
    int corners[4][2] = {
        { cell_box.x0 + 1, cell_box.z0 + 1 },
        { cell_box.x1 - 2, cell_box.z0 + 1 },
        { cell_box.x0 + 1, cell_box.z1 - 2 },
        { cell_box.x1 - 2, cell_box.z1 - 2 }
    };
    for (int i = 0; i < 4; i++) {
        if (mineshaft_rng_chance(rng, 0.5f)) continue;
        n += mineshaft_buffer_post(b, corners[i][0], floor_y, ceil_y,
                                   corners[i][1], cfg->mat_support);
    }

    // a torch on the center post so junctions aren't pitch black. sometimes.
    if (mineshaft_rng_chance(rng, 0.4f))
        n += mineshaft_buffer_add(b, cx, ceil_y - 1, cz, cfg->mat_torch);
    return n;
}
