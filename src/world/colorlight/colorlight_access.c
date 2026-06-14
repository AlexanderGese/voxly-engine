#include "colorlight_access.h"
#include "colorlight_grid.h"
#include "../block.h"
#include "../../config.h"

// resolve world coords to (chunk, local x, local z). returns NULL chunk if not
// loaded. y is already chunk-local since chunks span the full world height.
static chunk *resolve(world *w, int wx, int wz, int *lx, int *lz) {
    int cx, cz;
    world_to_chunk(wx, wz, &cx, &cz);
    world_to_local(wx, wz, lx, lz);
    return world_get_chunk(w, cx, cz);
}

colorlight_packed colorlight_world_get(world *w, int wx, int wy, int wz) {
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return 0;
    int lx, lz;
    chunk *c = resolve(w, wx, wz, &lx, &lz);
    if (!c) return 0;
    colorlight_grid *g = colorlight_grid_peek(c);
    if (!g) return 0;
    return colorlight_grid_get(g, lx, wy, lz);
}

void colorlight_world_set(world *w, int wx, int wy, int wz, colorlight_packed p) {
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return;
    int lx, lz;
    chunk *c = resolve(w, wx, wz, &lx, &lz);
    if (!c) return;
    colorlight_grid *g = colorlight_grid_for(c);
    if (!g) return;
    colorlight_grid_set(g, lx, wy, lz, p);
}

uint8_t colorlight_world_get_chan(world *w, int wx, int wy, int wz, int chan) {
    return colorlight_packed_chan(colorlight_world_get(w, wx, wy, wz), chan);
}

void colorlight_world_set_chan(world *w, int wx, int wy, int wz, int chan, uint8_t v) {
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return;
    int lx, lz;
    chunk *c = resolve(w, wx, wz, &lx, &lz);
    if (!c) return;
    colorlight_grid *g = colorlight_grid_for(c);
    if (!g) return;
    colorlight_grid_set_chan(g, lx, wy, lz, chan, v);
}

int colorlight_world_passable(world *w, int wx, int wy, int wz) {
    if (wy < 0 || wy >= CHUNK_SIZE_Y) return 0;
    block_id id = world_get_block(w, wx, wy, wz);
    return !block_is_opaque(id);
}
