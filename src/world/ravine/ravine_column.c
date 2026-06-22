#include "ravine_column.h"

#include "../block.h"

int ravine_column_surface(const chunk *c, int lx, int lz) {
    // walk down from the top, first solid-ish block is the surface. we treat
    // water as not-surface so a canyon that meets a lake still cuts the lakebed.
    for (int y = CHUNK_SIZE_Y - 1; y >= 0; y--) {
        block_id id = chunk_get_block(c, lx, y, lz);
        if (id == BLOCK_AIR || id == BLOCK_WATER) continue;
        return y;
    }
    return -1;
}

void ravine_column_scan(ravine_field *f, const chunk *c) {
    // centre footprint: read the real heights straight from the chunk.
    for (int lz = 0; lz < CHUNK_SIZE_Z; lz++) {
        for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
            int s = ravine_column_surface(c, lx, lz);
            if (s < 0) s = WORLD_SEA_LEVEL;   // empty column, sane fallback
            ravine_field_set_surface(f, lx + RAVINE_PAD, lz + RAVINE_PAD, s);
        }
    }

    // pad ring: extrapolate from the nearest centre-edge column. clamp the
    // local coord into [0, SIZE-1] and reuse that height. flat but seamless.
    for (int z = 0; z < RAVINE_DIM_Z; z++) {
        for (int x = 0; x < RAVINE_DIM_X; x++) {
            int in_centre = (x >= RAVINE_PAD && x < RAVINE_PAD + CHUNK_SIZE_X &&
                             z >= RAVINE_PAD && z < RAVINE_PAD + CHUNK_SIZE_Z);
            if (in_centre) continue;

            int lx = x - RAVINE_PAD;
            int lz = z - RAVINE_PAD;
            if (lx < 0) lx = 0; else if (lx >= CHUNK_SIZE_X) lx = CHUNK_SIZE_X - 1;
            if (lz < 0) lz = 0; else if (lz >= CHUNK_SIZE_Z) lz = CHUNK_SIZE_Z - 1;

            int s = ravine_column_surface(c, lx, lz);
            if (s < 0) s = WORLD_SEA_LEVEL;
            ravine_field_set_surface(f, x, z, s);
        }
    }
}
