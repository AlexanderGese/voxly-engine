#include "mb_snapshot.h"
#include "../../world/block.h"
static int snap_idx(int sx, int sy, int sz) {
    return sx + sz * MB_SNAP_W + sy * MB_SNAP_W * MB_SNAP_D;
}

static int in_snap(int sx, int sy, int sz) {
    return sx >= 0 && sx < MB_SNAP_W &&
           sy >= 0 && sy < MB_SNAP_H &&
           sz >= 0 && sz < MB_SNAP_D;
}

void mb_snapshot_fill(mb_snapshot *s, world *w, chunk *c) {
    s->base_x = c->cx * CHUNK_SIZE_X;
    s->base_z = c->cz * CHUNK_SIZE_Z;

    // sweep the padded volume. sy/sx/sz are snapshot coords; convert back to
    // world to read. cells above/below the world stay air with 0 light.
    for (int sy = 0; sy < MB_SNAP_H; sy++) {
        int wy = sy - 1;
        for (int sz = 0; sz < MB_SNAP_D; sz++) {
            int wz = s->base_z + sz - 1;
            for (int sx = 0; sx < MB_SNAP_W; sx++) {
                int wx = s->base_x + sx - 1;
                int idx = snap_idx(sx, sy, sz);

                if (wy < 0 || wy >= CHUNK_SIZE_Y) {
                    s->block[idx] = BLOCK_AIR;
                    s->light[idx] = 0;
                    continue;
                }

                s->block[idx] = world_get_block(w, wx, wy, wz);
                int sun = world_get_sunlight(w, wx, wy, wz);
                int blk = world_get_blocklight(w, wx, wy, wz);
                s->light[idx] = (uint8_t)(sun > blk ? sun : blk);
            }
        }
    }
}

// --- callbacks ---------------------------------------------------------------
// the builder passes WORLD coords;
translate to snapshot space and clamp. an
// out-of-skirt read (shouldnt happen with a correct 1-block skirt, but the
// greedy ao ring can reach 1 past a corner cell) returns air / no light.

static block_id snap_sample(void *ctx, int x, int y, int z) {
    const mb_snapshot *s = (const mb_snapshot *)ctx;
    int sx = x - s->base_x + 1;
    int sy = y + 1;
    int sz = z - s->base_z + 1;
    if (!in_snap(sx, sy, sz)) return BLOCK_AIR;
    return s->block[snap_idx(sx, sy, sz)];
}

static int snap_light(void *ctx, int x, int y, int z) {
    const mb_snapshot *s = (const mb_snapshot *)ctx;
int sx = x - s->base_x + 1;
int sy = y + 1;
int sz = z - s->base_z + 1;
if (!in_snap(sx, sy, sz)) return 0;
return s->light[snap_idx(sx, sy, sz)];
