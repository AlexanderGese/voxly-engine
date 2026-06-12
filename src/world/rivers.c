#include "rivers.h"
#include "block.h"
#include "worldgen.h"
#include "../math/noise.h"
#include "../config.h"

void rivers_carve(chunk *c, unsigned seed) {
    (void)seed;
    float s = 1.0f / 200.0f;
    for (int lz = 0; lz < CHUNK_SIZE_Z; lz++) {
        for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
            int wx = c->cx * CHUNK_SIZE_X + lx;
            int wz = c->cz * CHUNK_SIZE_Z + lz;

            float n = noise2d(wx * s, wz * s);
            // band near 0 = river channel
            if (n > -0.04f && n < 0.04f) {
                int h = worldgen_height_at(wx, wz, seed);
                for (int y = h - 2; y <= h; y++) {
                    if (y < 1) continue;
                    chunk_set_block(c, lx, y, lz, BLOCK_WATER);
                }
                chunk_set_block(c, lx, h + 1, lz, BLOCK_AIR);
            }
        }
    }
}
