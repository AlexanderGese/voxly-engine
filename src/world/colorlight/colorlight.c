#include "colorlight.h"
#include "colorlight_access.h"
#include "colorlight_grid.h"
#include "colorlight_queue.h"
#include "colorlight_emitter.h"
#include "colorlight_prop.h"
#include "colorlight_blend.h"
#include "../block.h"
#include "../../config.h"
#include "../../util/log.h"

void colorlight_recompute_chunk(world *w, chunk *c) {
    if (!c) return;
    colorlight_grid *g = colorlight_grid_for(c);
    if (!g) { LOGE("colorlight: no grid for chunk %d,%d", c->cx, c->cz); return; }

    // start clean. unlike the scalar path we don't try to be clever about
    // partial state on a full relight, just wipe and reseed.
    colorlight_grid_clear(g);
    colorlight_queue_reset(colorlight_queue_add());

    int wx0 = c->cx * CHUNK_SIZE_X;
    int wz0 = c->cz * CHUNK_SIZE_Z;

    // scan the chunk for emitters, seed each channel, then one big drain. we
    // seed straight into the grid here (not via prop_place) so all emitters go
    // in before any flooding, which gives correct max-blending where two lights
    // overlap.
    for (int y = 0; y < CHUNK_SIZE_Y; y++) {
        for (int lz = 0; lz < CHUNK_SIZE_Z; lz++) {
            for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
                block_id id = chunk_get_block(c, lx, y, lz);
                if (!colorlight_emitter_is(id)) continue;

                colorlight_packed seed = colorlight_emitter_seed(id);
                if (colorlight_packed_peak(seed) == 0) continue;

                int wx = wx0 + lx, wz = wz0 + lz;
                uint8_t r = colorlight_packed_r(seed);
                uint8_t gg = colorlight_packed_g(seed);
                uint8_t b = colorlight_packed_b(seed);

                if (r) { colorlight_grid_set_chan(g, lx, y, lz, 0, r);
                         colorlight_queue_push(colorlight_queue_add(), wx, y, wz, 0, r); }
                if (gg){ colorlight_grid_set_chan(g, lx, y, lz, 1, gg);
                         colorlight_queue_push(colorlight_queue_add(), wx, y, wz, 1, gg); }
                if (b) { colorlight_grid_set_chan(g, lx, y, lz, 2, b);
                         colorlight_queue_push(colorlight_queue_add(), wx, y, wz, 2, b); }
            }
        }
    }

    colorlight_prop_drain_add(w);
    g->dirty = 1;
    c->dirty = 1;
}

void colorlight_update_block(world *w, int wx, int wy, int wz) {
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return;

    block_id id = world_get_block(w, wx, wy, wz);

    if (block_is_opaque(id)) {
        // a wall went up here. any light that lived in this voxel has to go,
        // and the shadow behind it gets pulled.
        colorlight_prop_remove(w, wx, wy, wz);
        return;
    }

    if (colorlight_emitter_is(id)) {
        // new emitter (or one whose color changed). repaint from it. remove
        // first so a recolor doesn't leave the old hue smeared around.
        colorlight_prop_remove(w, wx, wy, wz);
        colorlight_prop_place(w, wx, wy, wz, colorlight_emitter_color(id));
        return;
    }

    // a transparent non-emitter (air, glass, water...) appeared where a wall
    // was. light should flow into the gap: pull whatever's there, then relight
    // from the brightest neighbor on each channel.
    colorlight_prop_remove(w, wx, wy, wz);

    static const int dx[6] = { 1,-1, 0, 0, 0, 0};
    static const int dy[6] = { 0, 0, 1,-1, 0, 0};
    static const int dz[6] = { 0, 0, 0, 0, 1,-1};
    colorlight_queue_reset(colorlight_queue_add());
    for (int d = 0; d < 6; d++) {
        int nx = wx + dx[d], ny = wy + dy[d], nz = wz + dz[d];
        if (ny < 0 || ny >= CHUNK_SIZE_Y) continue;
        colorlight_packed np = colorlight_world_get(w, nx, ny, nz);
        for (int ch = 0; ch < 3; ch++) {
            uint8_t lv = colorlight_packed_chan(np, ch);
            if (lv > 1) colorlight_queue_push(colorlight_queue_add(), nx, ny, nz, (uint8_t)ch, lv);
        }
    }
    colorlight_prop_drain_add(w);
}

colorlight_rgb colorlight_sample(world *w, int wx, int wy, int wz, uint8_t sun) {
    colorlight_packed blk = colorlight_world_get(w, wx, wy, wz);
    // small cold ambient floor so unlit caves read as near-black, not pure 0.
    colorlight_rgb amb = { 6, 7, 10 };
    return colorlight_blend_ambient(sun, blk, amb);
}
