#include "oregen_stamp.h"
#include "../../config.h"

int oregen_stamp_default_replace(block_id existing) {
    // only burrow into plain stone. everything else stays put.
    return existing == BLOCK_STONE;
}

int oregen_stamp_chunk(chunk *c, const oregen_buf *buf) {
    if (!c || !buf || !buf->items) return 0;

    int base_x = c->cx * CHUNK_SIZE_X;
    int base_z = c->cz * CHUNK_SIZE_Z;
    int wrote = 0;

    for (int i = 0; i < buf->count; i++) {
        const oregen_cell *cell = &buf->items[i];

        int lx = cell->x - base_x;
        int lz = cell->z - base_z;
        int ly = cell->y;

        // skip cells that belong to a neighbor chunk; they get stamped when
        // that chunk runs its own pass.
        if (lx < 0 || lx >= CHUNK_SIZE_X) continue;
        if (lz < 0 || lz >= CHUNK_SIZE_Z) continue;
        if (ly < 0 || ly >= CHUNK_SIZE_Y) continue;

        block_id had = chunk_get_block(c, lx, ly, lz);
        if (!oregen_stamp_default_replace(had)) continue;

        chunk_set_block(c, lx, ly, lz, cell->id);
        wrote++;
    }

    if (wrote > 0) c->dirty = 1;   // needs a remesh now
    return wrote;
}

int oregen_apply_chunk(chunk *c, oregen_buf *buf, int surface_y,
                       uint32_t world_seed) {
    if (!c || !buf) return 0;

    int origin_x = c->cx * CHUNK_SIZE_X;
    int origin_z = c->cz * CHUNK_SIZE_Z;

    oregen_buf_reset(buf);
    oregen_params params = oregen_params_default(world_seed);
    oregen_generate_chunk(buf, origin_x, origin_z, surface_y, &params);

    return oregen_stamp_chunk(c, buf);
}
