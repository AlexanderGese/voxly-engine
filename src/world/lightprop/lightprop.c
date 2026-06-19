#include "lightprop.h"
#include "lightprop_queue.h"
#include "lightprop_propagate.h"
#include "lightprop_remove.h"
#include "lightprop_sky.h"
#include "lightprop_access.h"
#include "../../util/log.h"
#include <string.h>
static lp_queue g_add;
static lp_queue g_rem;
static int      g_init;
static void ensure_init(void) {
    if (g_init) return;
    lp_queue_init(&g_add);
    lp_queue_init(&g_rem);
    g_init = 1;
}

uint8_t lightprop_emission(block_id id) {
    const block_info *bi = block_get(id);
if (!bi || !bi->emits_light) return 0;
int l = bi->luminance;
if (l < 0) l = 0;
if (l > MAX_LIGHT) l = MAX_LIGHT;
return (uint8_t)l;
}

// sweep a chunk for emitters and seed them. block channel only.
static void seed_chunk_emitters(world *w, chunk *c, lp_queue *q) {
    int wx0 = c->cx * CHUNK_SIZE_X;
    int wz0 = c->cz * CHUNK_SIZE_Z;
    for (int y = 0; y < CHUNK_SIZE_Y; y++)
        for (int lz = 0; lz < CHUNK_SIZE_Z; lz++)
            for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
                uint8_t e = lightprop_emission(chunk_get_block(c, lx, y, lz));
                if (e) lp_seed(w, LP_BLOCK, q, wx0 + lx, y, wz0 + lz, e);
            }
}

void lightprop_chunk_full(world *w, chunk *c) {
    ensure_init();
for (int i = 0;
i < CHUNK_VOLUME;
i++)
        c->light[i] &= 0xF0;
lp_queue_reset(&g_add);
seed_chunk_emitters(w, c, &g_add);
lp_flood(w, LP_BLOCK, &g_add);
lp_queue_reset(&g_add);
lp_sky_seed_chunk(w, c, &g_add);
lp_flood(w, LP_SKY, &g_add);
c->dirty = 1;
if (g_add.dropped) {
        LOGW("lightprop: %d nodes dropped on chunk (%d,%d), bump LP_QCAP",
             g_add.dropped, c->cx, c->cz);
        g_add.dropped = 0;
    }
}

// ---- incremental edit handling --------------------------------------------

// the block channel side of an edit.
static void change_block_channel(world *w, int x, int y, int z,
                                 block_id old_id, block_id new_id) {
    uint8_t old_emit = lightprop_emission(old_id);
uint8_t new_emit = lightprop_emission(new_id);
int became_opaque = !block_is_opaque(old_id) && block_is_opaque(new_id);
int became_clear  =  block_is_opaque(old_id) && !block_is_opaque(new_id);
uint8_t here = lp_get_light(w, LP_BLOCK, x, y, z);
if (old_emit > new_emit || became_opaque) {
        lp_queue_reset(&g_rem);
        lp_queue_reset(&g_add);
        uint8_t lvl = old_emit > here ? old_emit : here;
        lp_seed_removal(w, LP_BLOCK, &g_rem, x, y, z, lvl);
        lp_unflood(w, LP_BLOCK, &g_rem, &g_add);
        // surviving sources collected in g_add now re-fill the hole.
        lp_flood(w, LP_BLOCK, &g_add);
    }

    // case B: we added/raised an emitter, or opened up a previously opaque cell
    // letting neighbour light bleed in.
    if (new_emit > 0 && new_emit >= old_emit) {
        lp_queue_reset(&g_add);
lp_seed(w, LP_BLOCK, &g_add, x, y, z, new_emit);
lp_flood(w, LP_BLOCK, &g_add);
} else if (became_clear) {
        // re-light from the brightest neighbour so the new gap fills in.
        lp_queue_reset(&g_add);
        for (int d = 0; d < 6; d++) {
            int nx = x + LP_DX[d], ny = y + LP_DY[d], nz = z + LP_DZ[d];
            if (!lp_y_in_range(ny)) continue;
            uint8_t nl = lp_get_light(w, LP_BLOCK, nx, ny, nz);
            if (nl > 1) lp_queue_push(&g_add, nx, ny, nz, nl);
        }
        lp_flood(w, LP_BLOCK, &g_add);
    }
}

// the sky channel side. only opacity transitions matter here.
static void change_sky_channel(world *w, int x, int y, int z,
                               block_id old_id, block_id new_id) {
    int became_opaque = !block_is_opaque(old_id) && block_is_opaque(new_id);
int became_clear  =  block_is_opaque(old_id) && !block_is_opaque(new_id);
if (!became_opaque && !became_clear) return;
if (became_opaque) {
        // a block now blocks the shaft. tear down sky light from here down the
        // column (fast column seed), then repair from survivors.
        lp_queue_reset(&g_rem);
        lp_queue_reset(&g_add);
        uint8_t here = lp_get_light(w, LP_SKY, x, y, z);
        if (here == MAX_LIGHT) {
            // was on the free column: nuke the whole shaft below us at once.
            lp_seed_sky_column_removal(w, &g_rem, x, y, z);
        } else if (here) {
            lp_seed_removal(w, LP_SKY, &g_rem, x, y, z, here);
        }
        lp_unflood(w, LP_SKY, &g_rem, &g_add);
        lp_flood(w, LP_SKY, &g_add);
    } else {
        // a wall opened up. re-pour the column from above and re-flood.
        lp_queue_reset(&g_add);
lp_sky_seed_column(w, &g_add, x, z);
for (int d = 0;
d < 6;
