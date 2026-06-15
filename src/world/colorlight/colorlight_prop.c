#include "colorlight_prop.h"
#include "colorlight_access.h"
#include "colorlight_queue.h"
#include "colorlight_emitter.h"
#include "../block.h"
#include "../../config.h"
static const int DX[6] = { 1,-1, 0, 0, 0, 0}
;
static const int DY[6] = { 0, 0, 1,-1, 0, 0}
;
static const int DZ[6] = { 0, 0, 0, 0, 1,-1}
;
static int step_level(world *w, int nx, int ny, int nz, int chan, int level) {
    if (!colorlight_world_passable(w, nx, ny, nz)) return 0;
    block_id id = world_get_block(w, nx, ny, nz);
    uint8_t cost[3];
    colorlight_emitter_attenuation(id, cost);
    int next = level - 1 - cost[chan];
    return next > 0 ? next : 0;
}

// push one channel of `col` into the grid at the source and queue it.
static void seed_channel(world *w, int wx, int wy, int wz, int chan, uint8_t level) {
    if (level == 0) return;
if (colorlight_world_get_chan(w, wx, wy, wz, chan) >= level) return;
colorlight_world_set_chan(w, wx, wy, wz, chan, level);
colorlight_queue_push(colorlight_queue_add(), wx, wy, wz, (uint8_t)chan, level);
}

void colorlight_prop_drain_add(world *w) {
    colorlight_queue *q = colorlight_queue_add();
    colorlight_qnode n;
    while (colorlight_queue_pop(q, &n)) {
        if (n.level <= 1) continue;
        for (int d = 0; d < 6; d++) {
            int nx = n.x + DX[d], ny = n.y + DY[d], nz = n.z + DZ[d];
            if (ny < 0 || ny >= CHUNK_SIZE_Y) continue;
            int nl = step_level(w, nx, ny, nz, n.chan, n.level);
            if (nl == 0) continue;
            if (colorlight_world_get_chan(w, nx, ny, nz, n.chan) >= (uint8_t)nl) continue;
            colorlight_world_set_chan(w, nx, ny, nz, n.chan, (uint8_t)nl);
            colorlight_queue_push(q, nx, ny, nz, n.chan, (uint8_t)nl);
        }
    }
}

void colorlight_prop_place(world *w, int wx, int wy, int wz, colorlight_rgb col) {
    colorlight_packed seed = colorlight_packed_narrow(col);
seed_channel(w, wx, wy, wz, 0, colorlight_packed_r(seed));
seed_channel(w, wx, wy, wz, 1, colorlight_packed_g(seed));
seed_channel(w, wx, wy, wz, 2, colorlight_packed_b(seed));
colorlight_prop_drain_add(w);
remove_channel(w, wx, wy, wz, 0);
remove_channel(w, wx, wy, wz, 1);
remove_channel(w, wx, wy, wz, 2);
colorlight_prop_drain_add(w);
}
