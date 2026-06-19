#include "lightprop_border.h"
#include "lightprop_propagate.h"
#include "lightprop_access.h"
#include "lightprop_step.h"
#include "../../util/log.h"

// one scratch queue for seam work. like the main driver, lighting is single
// threaded so a file-static is fine.
static lp_queue g_seam;
static int      g_seam_init;

static void seam_init(void) {
    if (!g_seam_init) { lp_queue_init(&g_seam); g_seam_init = 1; }
}

// enumerate the cells along chunk `c`'s face in direction `face`. fills wx/wz to
// the world coord of the cell ON OUR SIDE of the seam; the neighbour cell is one
// step further in the face normal. returns the run length along the face axis.
static int face_strip(const chunk *c, int face, int idx, int *wx, int *wz) {
    int wx0 = c->cx * CHUNK_SIZE_X;
    int wz0 = c->cz * CHUNK_SIZE_Z;
    switch (face) {
        case 0: *wx = wx0 + CHUNK_SIZE_X - 1; *wz = wz0 + idx; return CHUNK_SIZE_Z; // +x
        case 1: *wx = wx0;                    *wz = wz0 + idx; return CHUNK_SIZE_Z; // -x
        case 4: *wx = wx0 + idx; *wz = wz0 + CHUNK_SIZE_Z - 1; return CHUNK_SIZE_X; // +z
        case 5: *wx = wx0 + idx; *wz = wz0;                    return CHUNK_SIZE_X; // -z
        default: return 0;
    }
}

// seed the seam in both directions for one channel, then flood once.
static void stitch_channel(world *w, chunk *c, int face, lp_channel ch) {
    int dx = LP_DX[face], dz = LP_DZ[face];
    int tx, tz;
    int run = face_strip(c, face, 0, &tx, &tz);
    (void)tx; (void)tz;
    if (run == 0) return;

    lp_queue_reset(&g_seam);
    for (int i = 0; i < run; i++) {
        int wx, wz;
        face_strip(c, face, i, &wx, &wz);
        for (int y = 0; y < CHUNK_SIZE_Y; y++) {
            // our cell and the neighbour cell across the seam.
            int ox = wx + dx, oz = wz + dz;
            if (!lp_cell_loaded(w, ox, oz)) continue;

            uint8_t mine  = lp_get_light(w, ch, wx, y, wz);
            uint8_t other = lp_get_light(w, ch, ox, y, oz);

            // whichever side is brighter seeds; flood will carry it across.
            if (mine > 1) lp_queue_push(&g_seam, wx, y, wz, mine);
            if (other > 1) lp_queue_push(&g_seam, ox, y, oz, other);
        }
    }
    lp_flood(w, ch, &g_seam);
}

void lp_border_stitch(world *w, chunk *c, int face) {
    seam_init();
    if (face != 0 && face != 1 && face != 4 && face != 5) return;
    int nx = c->cx + LP_DX[face];
    int nz = c->cz + LP_DZ[face];
    if (!world_get_chunk(w, nx, nz)) return;   // neighbour not here yet

    stitch_channel(w, c, face, LP_BLOCK);
    stitch_channel(w, c, face, LP_SKY);
    c->dirty = 1;
}

void lp_border_stitch_all(world *w, chunk *c) {
    lp_border_stitch(w, c, 0);
    lp_border_stitch(w, c, 1);
    lp_border_stitch(w, c, 4);
    lp_border_stitch(w, c, 5);
}

int lp_border_pull(world *w, chunk *c) {
    seam_init();
    int brightened = 0;
    static const int faces[4] = { 0, 1, 4, 5 };

    for (int fi = 0; fi < 4; fi++) {
        int face = faces[fi];
        int dx = LP_DX[face], dz = LP_DZ[face];
        int tx, tz;
        int run = face_strip(c, face, 0, &tx, &tz);
        (void)tx; (void)tz;

        lp_queue_reset(&g_seam);
        for (int i = 0; i < run; i++) {
            int wx, wz;
            face_strip(c, face, i, &wx, &wz);
            int ox = wx + dx, oz = wz + dz;
            if (!lp_cell_loaded(w, ox, oz)) continue;

            for (int y = 0; y < CHUNK_SIZE_Y; y++) {
                // what would the neighbour cell push into our edge cell?
                block_id id = lp_get_block(w, wx, y, wz);
                if (!lp_transmits(id)) continue;
                // opposite-direction step (face^1) carries light back into us.
                uint8_t nb_blk = lp_get_light(w, LP_BLOCK, ox, y, oz);
                uint8_t nb_sky = lp_get_light(w, LP_SKY,   ox, y, oz);
                uint8_t gb = lp_step_propagate(LP_BLOCK, face ^ 1, nb_blk, id);
                uint8_t gs = lp_step_propagate(LP_SKY,   face ^ 1, nb_sky, id);
                if (gb > lp_get_light(w, LP_BLOCK, wx, y, wz)) {
                    lp_set_light(w, LP_BLOCK, wx, y, wz, gb);
                    lp_queue_push(&g_seam, wx, y, wz, gb);
                    brightened++;
                }
                if (gs > lp_get_light(w, LP_SKY, wx, y, wz)) {
                    lp_set_light(w, LP_SKY, wx, y, wz, gs);
                    brightened++;
                }
            }
        }
        // block-channel seeds we just pushed still need to spread inward.
        lp_flood(w, LP_BLOCK, &g_seam);
    }

    if (brightened) c->dirty = 1;
    return brightened;
}
