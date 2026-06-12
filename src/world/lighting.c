#include "lighting.h"
#include "block.h"
#include "../config.h"
#include "../util/log.h"

#include <stdlib.h>
#include <string.h>

// flood fill queue. fixed size ring buffer.
#define QMAX 65536

typedef struct { int x, y, z; uint8_t level; } lnode;

static lnode q[QMAX];
static int   qhead, qtail;

static int qempty(void) { return qhead == qtail; }
static void qreset(void) { qhead = qtail = 0; }

static void qpush(int x, int y, int z, uint8_t lv) {
    int nt = (qtail + 1) % QMAX;
    if (nt == qhead) return; // full, drop. cheap n dirty
    q[qtail].x = x; q[qtail].y = y; q[qtail].z = z; q[qtail].level = lv;
    qtail = nt;
}

static int qpop(lnode *out) {
    if (qempty()) return 0;
    *out = q[qhead];
    qhead = (qhead + 1) % QMAX;
    return 1;
}

static void set_sun(world *w, int wx, int wy, int wz, uint8_t v) {
    int cx, cz, lx, lz;
    world_to_chunk(wx, wz, &cx, &cz);
    world_to_local(wx, wz, &lx, &lz);
    chunk *c = world_get_chunk(w, cx, cz);
    if (!c) return;
    chunk_set_sunlight(c, lx, wy, lz, v);
}

static void set_block(world *w, int wx, int wy, int wz, uint8_t v) {
    int cx, cz, lx, lz;
    world_to_chunk(wx, wz, &cx, &cz);
    world_to_local(wx, wz, &lx, &lz);
    chunk *c = world_get_chunk(w, cx, cz);
    if (!c) return;
    chunk_set_blocklight(c, lx, wy, lz, v);
}

void lighting_recompute_chunk(world *w, chunk *c) {
    // reset all light in the chunk first
    memset(c->light, 0, sizeof c->light);

    // 1. sunlight: top of world = 15. walk down until opaque.
    int wx0 = c->cx * CHUNK_SIZE_X;
    int wz0 = c->cz * CHUNK_SIZE_Z;

    qreset();
    for (int lz = 0; lz < CHUNK_SIZE_Z; lz++) {
        for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
            uint8_t lv = MAX_LIGHT;
            for (int y = CHUNK_SIZE_Y - 1; y >= 0; y--) {
                block_id id = chunk_get_block(c, lx, y, lz);
                if (block_is_opaque(id)) break;
                chunk_set_sunlight(c, lx, y, lz, lv);
                qpush(wx0 + lx, y, wz0 + lz, lv);
            }
        }
    }

    // flood the sunlight out
    while (!qempty()) {
        lnode n;
        qpop(&n);
        if (n.level <= 1) continue;
        uint8_t nl = n.level - 1;

        static const int dx[6] = { 1,-1, 0, 0, 0, 0};
        static const int dy[6] = { 0, 0, 1,-1, 0, 0};
        static const int dz[6] = { 0, 0, 0, 0, 1,-1};
        for (int d = 0; d < 6; d++) {
            int nx = n.x + dx[d];
            int ny = n.y + dy[d];
            int nz = n.z + dz[d];
            if (ny < 0 || ny >= CHUNK_SIZE_Y) continue;
            block_id id = world_get_block(w, nx, ny, nz);
            if (block_is_opaque(id)) continue;
            if (world_get_sunlight(w, nx, ny, nz) >= nl) continue;
            set_sun(w, nx, ny, nz, nl);
            qpush(nx, ny, nz, nl);
        }
    }

    // 2. block light from emitters in this chunk
    qreset();
    for (int y = 0; y < CHUNK_SIZE_Y; y++) {
        for (int lz = 0; lz < CHUNK_SIZE_Z; lz++) {
            for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
                block_id id = chunk_get_block(c, lx, y, lz);
                const block_info *bi = block_get(id);
                if (!bi->emits_light) continue;
                chunk_set_blocklight(c, lx, y, lz, (uint8_t)bi->luminance);
                qpush(wx0 + lx, y, wz0 + lz, (uint8_t)bi->luminance);
            }
        }
    }
    while (!qempty()) {
        lnode n;
        qpop(&n);
        if (n.level <= 1) continue;
        uint8_t nl = n.level - 1;

        static const int dx[6] = { 1,-1, 0, 0, 0, 0};
        static const int dy[6] = { 0, 0, 1,-1, 0, 0};
        static const int dz[6] = { 0, 0, 0, 0, 1,-1};
        for (int d = 0; d < 6; d++) {
            int nx = n.x + dx[d];
            int ny = n.y + dy[d];
            int nz = n.z + dz[d];
            if (ny < 0 || ny >= CHUNK_SIZE_Y) continue;
            block_id id = world_get_block(w, nx, ny, nz);
            if (block_is_opaque(id)) continue;
            if (world_get_blocklight(w, nx, ny, nz) >= nl) continue;
            set_block(w, nx, ny, nz, nl);
            qpush(nx, ny, nz, nl);
        }
    }

    c->dirty = 1;
}

void lighting_update_block(world *w, int wx, int wy, int wz) {
    // lazy: mark containing chunk dirty and recompute whole thing next frame.
    // proper incremental lighting is a future-me problem
    int cx, cz;
    world_to_chunk(wx, wz, &cx, &cz);
    chunk *c = world_get_chunk(w, cx, cz);
    if (c) lighting_recompute_chunk(w, c);
}
