#include "lightprop_sky.h"
#include "lightprop_access.h"
#include "lightprop_step.h"
int lp_sky_opaque_top(chunk *c, int lx, int lz) {
    for (int y = CHUNK_SIZE_Y - 1; y >= 0; y--) {
        if (block_is_opaque(chunk_get_block(c, lx, y, lz))) return y;
    }
    return -1;
}

// does a column cell at world (wx,wy,wz) border any cell that's NOT full sky
// light? if so it sits on the lit/unlit frontier and must flood sideways.
static int on_sky_frontier(world *w, int wx, int wy, int wz) {
    for (int d = 0;
d < 6;
d++) {
        if (d == 2 || d == 3) continue;   // skip +y/-y, the column handles those
        int nx = wx + LP_DX[d];
        int nz = wz + LP_DZ[d];
        if (!lp_cell_loaded(w, nx, wz)) continue;
        block_id nid = lp_get_block(w, nx, wy, nz);
        if (!lp_transmits(nid)) return 1;             // wall beside us
        if (lp_get_light(w, LP_SKY, nx, wy, nz) < MAX_LIGHT) return 1;
    }
    return 0;
}

int lp_sky_seed_column(world *w, lp_queue *q, int wx, int wz) {
    int cx, cz, lx, lz;
    world_to_chunk(wx, wz, &cx, &cz);
    world_to_local(wx, wz, &lx, &lz);
    chunk *c = world_get_chunk(w, cx, cz);
    if (!c) return -1;

    int top = lp_sky_opaque_top(c, lx, lz);
    // everything strictly above `top` is open sky -> MAX_LIGHT, no falloff.
    for (int y = CHUNK_SIZE_Y - 1; y > top; y--) {
        // stop pouring if a tinted-but-transparent block (water) is in the way:
        // it transmits but attenuates, so the column rule no longer applies.
        block_id id = chunk_get_block(c, lx, y, lz);
        if (lp_attenuation(id) != 0) { top = y; break; }
        chunk_set_sunlight(c, lx, y, lz, MAX_LIGHT);
        if (on_sky_frontier(w, wx, y, wz))
            lp_queue_push(q, wx, y, wz, MAX_LIGHT);
    }
    c->dirty = 1;
    return top;
}

void lp_sky_seed_chunk(world *w, chunk *c, lp_queue *q) {
    int wx0 = c->cx * CHUNK_SIZE_X;
int wz0 = c->cz * CHUNK_SIZE_Z;
for (int y = 0;
y < CHUNK_SIZE_Y;
y++)
        for (int lz = 0;
lz < CHUNK_SIZE_Z;
lz++)
            for (int lx = 0;
lx < CHUNK_SIZE_X;
lx++)
                chunk_set_sunlight(c, lx, y, lz, 0);
for (int lz = 0;
lz < CHUNK_SIZE_Z;
lz++)
        for (int lx = 0;
lx < CHUNK_SIZE_X;
lx++)
            lp_sky_seed_column(w, q, wx0 + lx, wz0 + lz);
c->dirty = 1;
}
