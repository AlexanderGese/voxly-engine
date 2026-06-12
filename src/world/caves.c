#include "caves.h"
#include "block.h"
#include "../math/noise.h"
#include "../config.h"

void caves_carve(chunk *c, unsigned seed) {
    (void)seed;
    float s = 1.0f / 24.0f;
    for (int y = 1; y < WORLD_SEA_LEVEL + 4; y++) {
        for (int z = 0; z < CHUNK_SIZE_Z; z++) {
            for (int x = 0; x < CHUNK_SIZE_X; x++) {
                int wx = c->cx * CHUNK_SIZE_X + x;
                int wz = c->cz * CHUNK_SIZE_Z + z;
                float n = noise3d(wx * s, y * s * 1.5f, wz * s);
                if (n > 0.55f) {
                    block_id cur = chunk_get_block(c, x, y, z);
                    if (cur == BLOCK_STONE || cur == BLOCK_DIRT) {
                        chunk_set_block(c, x, y, z, BLOCK_AIR);
                    }
                }
            }
        }
    }
}
