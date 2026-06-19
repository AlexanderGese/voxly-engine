#include "lightprop_access.h"

#include <stddef.h>

// resolve a world cell to (chunk, local). returns NULL chunk if not loaded.
static chunk *resolve(world *w, int wx, int wz, int *lx, int *lz) {
    int cx, cz;
    world_to_chunk(wx, wz, &cx, &cz);
    world_to_local(wx, wz, lx, lz);
    return world_get_chunk(w, cx, cz);
}

uint8_t lp_get_light(world *w, lp_channel ch, int wx, int wy, int wz) {
    if (!lp_y_in_range(wy)) return 0;
    int lx, lz;
    chunk *c = resolve(w, wx, wz, &lx, &lz);
    if (!c) return 0;
    return ch == LP_SKY ? chunk_get_sunlight(c, lx, wy, lz)
                        : chunk_get_blocklight(c, lx, wy, lz);
}

void lp_set_light(world *w, lp_channel ch, int wx, int wy, int wz, uint8_t v) {
    if (!lp_y_in_range(wy)) return;
    int lx, lz;
    chunk *c = resolve(w, wx, wz, &lx, &lz);
    if (!c) return;
    if (ch == LP_SKY) chunk_set_sunlight(c, lx, wy, lz, v);
    else              chunk_set_blocklight(c, lx, wy, lz, v);
    c->dirty = 1;
}

block_id lp_get_block(world *w, int wx, int wy, int wz) {
    if (!lp_y_in_range(wy)) return BLOCK_BEDROCK; // treat out-of-world as solid
    int lx, lz;
    chunk *c = resolve(w, wx, wz, &lx, &lz);
    if (!c) return BLOCK_BEDROCK; // unloaded -> opaque, light won't escape
    return chunk_get_block(c, lx, wy, lz);
}

int lp_cell_loaded(world *w, int wx, int wz) {
    int cx, cz;
    world_to_chunk(wx, wz, &cx, &cz);
    return world_get_chunk(w, cx, cz) != NULL;
}

void lp_mark_dirty(world *w, int wx, int wz) {
    int cx, cz;
    world_to_chunk(wx, wz, &cx, &cz);
    chunk *c = world_get_chunk(w, cx, cz);
    if (c) c->dirty = 1;
}
