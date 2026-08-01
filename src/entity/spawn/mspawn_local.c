#include "mspawn_local.h"
#include "mspawn_ground.h"
#include "../../world/block.h"
#include "../../math/vec3.h"

// is there `need` blocks of non-solid air above (x, y, z) within the chunk?
// if the column runs out the top of the chunk we count that as open sky, which
// is fine for spawn purposes.
static int local_headroom(const chunk *c, int x, int y, int z, int need) {
    for (int i = 1; i <= need; i++) {
        int wy = y + i;
        if (wy >= CHUNK_SIZE_Y) return 1;    // hit the ceiling, call it open
        if (block_is_solid(chunk_get_block(c, x, wy, z))) return 0;
    }
    return 1;
}

void mspawn_local_build(mspawn_local *lc, const chunk *c) {
    lc->cx    = c->cx;
    lc->cz    = c->cz;
    lc->count = 0;
    lc->valid = 1;

    for (int x = 0; x < CHUNK_SIZE_X; x++) {
        for (int z = 0; z < CHUNK_SIZE_Z; z++) {
            // walk down from the top until we meet the first solid block; the
            // air directly above it (if any) is the standing spot.
            int found = 0;
            for (int y = CHUNK_SIZE_Y - 2; y >= 1 && !found; y--) {
                block_id here  = chunk_get_block(c, x, y, z);
                block_id above = chunk_get_block(c, x, y + 1, z);
                if (!block_is_solid(here)) continue;
                if (block_is_solid(above)) continue;     // buried
                if (here == BLOCK_WATER || above == BLOCK_WATER) continue;
                if (!local_headroom(c, x, y, z, MSPAWN_HEADROOM)) continue;

                if (lc->count < MSPAWN_LOCAL_MAX) {
                    mspawn_spot *s = &lc->spots[lc->count++];
                    s->lx  = (uint8_t)x;
                    s->lz  = (uint8_t)z;
                    s->fy  = (uint8_t)y;
                    s->sun = chunk_get_sunlight(c, x, y + 1, z);
                }
                found = 1;     // one spot per column, the topmost
            }
        }
    }
}

vec3 mspawn_local_pos(const mspawn_local *lc, int i) {
    if (i < 0 || i >= lc->count) return VEC3_ZERO;
    const mspawn_spot *s = &lc->spots[i];
    int wx = lc->cx * CHUNK_SIZE_X + s->lx;
    int wz = lc->cz * CHUNK_SIZE_Z + s->lz;
    return vec3_new((float)wx + 0.5f, (float)(s->fy + 1), (float)wz + 0.5f);
}

int mspawn_local_spot(const mspawn_local *lc, int i, int *out_fy, int *out_sun) {
    if (i < 0 || i >= lc->count) return 0;
    if (out_fy)  *out_fy  = lc->spots[i].fy;
    if (out_sun) *out_sun = lc->spots[i].sun;
    return 1;
}
