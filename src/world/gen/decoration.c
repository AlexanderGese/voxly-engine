#include "decoration.h"
#include "../block.h"
#include "../block_ext.h"
#include "../worldgen.h"
#include "../../math/rng.h"
#include "../../config.h"

void decoration_scatter(chunk *c, unsigned seed) {
    rng r;
    rng_init(&r, seed ^ ((uint64_t)c->cx * 99991u) ^ ((uint64_t)c->cz * 99989u));

    for (int lz = 0; lz < CHUNK_SIZE_Z; lz++) {
        for (int lx = 0; lx < CHUNK_SIZE_X; lx++) {
            int wx = c->cx * CHUNK_SIZE_X + lx;
            int wz = c->cz * CHUNK_SIZE_Z + lz;
            int h  = worldgen_height_at(wx, wz, seed);
            if (h <= WORLD_SEA_LEVEL + 1 || h >= CHUNK_SIZE_Y - 2) continue;

            block_id surface = chunk_get_block(c, lx, h, lz);
            block_id above   = chunk_get_block(c, lx, h + 1, lz);
            if (above != BLOCK_AIR) continue;

            float roll = rng_float01(&r);

            if (surface == BLOCK_GRASS) {
                if (roll < 0.03f) {
                    chunk_set_block(c, lx, h + 1, lz, BLOCK_FLOWER_RED);
                } else if (roll < 0.06f) {
                    chunk_set_block(c, lx, h + 1, lz, BLOCK_FLOWER_YELLOW);
                } else if (roll < 0.15f) {
                    chunk_set_block(c, lx, h + 1, lz, BLOCK_TALL_GRASS);
                }
            }

            if (surface == BLOCK_SAND && h == WORLD_SEA_LEVEL + 1) {
                if (roll < 0.02f) {
                    // sugarcane — stack 1-3 high
                    int height = rng_range(&r, 1, 3);
                    for (int dy = 1; dy <= height; dy++) {
                        if (h + dy >= CHUNK_SIZE_Y) break;
                        chunk_set_block(c, lx, h + dy, lz, BLOCK_SUGARCANE);
                    }
                } else if (roll < 0.01f) {
                    // cactus — 1-3 high, no neighbors
                    int ok = 1;
                    if (lx > 0 && chunk_get_block(c, lx - 1, h + 1, lz) != BLOCK_AIR) ok = 0;
                    if (lx < CHUNK_SIZE_X-1 && chunk_get_block(c, lx+1, h+1, lz) != BLOCK_AIR) ok = 0;
                    if (lz > 0 && chunk_get_block(c, lx, h + 1, lz-1) != BLOCK_AIR) ok = 0;
                    if (lz < CHUNK_SIZE_Z-1 && chunk_get_block(c, lx, h+1, lz+1) != BLOCK_AIR) ok = 0;
                    if (ok) {
                        int height = rng_range(&r, 1, 3);
                        for (int dy = 1; dy <= height; dy++) {
                            chunk_set_block(c, lx, h + dy, lz, BLOCK_CACTUS);
                        }
                    }
                }
            }

            // mushrooms in dark spots (caves or tree shade)
            if (surface == BLOCK_STONE || surface == BLOCK_DIRT) {
                if (h < WORLD_SEA_LEVEL - 5 && roll < 0.05f) {
                    chunk_set_block(c, lx, h + 1, lz,
                        rng_float01(&r) < 0.5f ? BLOCK_MUSHROOM_RED : BLOCK_MUSHROOM_BROWN);
                }
            }
        }
    }
}
