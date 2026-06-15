#include "colorlight_prop.h"
#include "colorlight_access.h"
#include "colorlight_queue.h"
#include "colorlight_emitter.h"
#include "../block.h"
#include "../../config.h"

// 6-neighborhood, same order lighting.c uses.
static const int DX[6] = { 1,-1, 0, 0, 0, 0};
static const int DY[6] = { 0, 0, 1,-1, 0, 0};
static const int DZ[6] = { 0, 0, 0, 0, 1,-1};

// what one channel can reach stepping from `level` into the block AT (nx,ny,nz).
// normal cost is 1; translucent media add their per-channel attenuation. returns
// 0 if the step dies here (or the target is opaque).
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
}

// remove one channel. classic two-pass: walk the dark front pulling down any
// neighbor that was lit purely by us (its level is strictly less than ours, so
// it could only have come from this source), and stash boundary survivors to
// relight from afterwards.
static void remove_channel(world *w, int wx, int wy, int wz, int chan) {
    uint8_t start = colorlight_world_get_chan(w, wx, wy, wz, chan);
    if (start == 0) return;

    colorlight_queue *rq = colorlight_queue_remove();
    colorlight_queue *aq = colorlight_queue_add();
    colorlight_queue_reset(rq);

    colorlight_world_set_chan(w, wx, wy, wz, chan, 0);
    colorlight_queue_push(rq, wx, wy, wz, (uint8_t)chan, start);

    colorlight_qnode n;
    while (colorlight_queue_pop(rq, &n)) {
        for (int d = 0; d < 6; d++) {
            int nx = n.x + DX[d], ny = n.y + DY[d], nz = n.z + DZ[d];
            if (ny < 0 || ny >= CHUNK_SIZE_Y) continue;
            uint8_t here = colorlight_world_get_chan(w, nx, ny, nz, chan);
            if (here == 0) continue;
            if (here < n.level) {
                // this neighbor was downstream of us; kill it and keep walking.
                colorlight_world_set_chan(w, nx, ny, nz, chan, 0);
                colorlight_queue_push(rq, nx, ny, nz, (uint8_t)chan, here);
            } else {
                // brighter or equal: it has its own source. relight from it.
                colorlight_queue_push(aq, nx, ny, nz, (uint8_t)chan, here);
            }
        }
    }
}

void colorlight_prop_remove(world *w, int wx, int wy, int wz) {
    colorlight_queue_reset(colorlight_queue_add());
    // tear down all three channels first, collecting survivors into the add
    // queue, then a single relight drains them.
    remove_channel(w, wx, wy, wz, 0);
    remove_channel(w, wx, wy, wz, 1);
    remove_channel(w, wx, wy, wz, 2);
    colorlight_prop_drain_add(w);
}
