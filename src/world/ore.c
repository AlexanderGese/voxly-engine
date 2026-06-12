#include "ore.h"
#include "block.h"
#include "../math/rng.h"
#include "../config.h"
static void place_vein(chunk *c, rng *r, int cx, int cy, int cz, int size, block_id id) {
    for (int i = 0; i < size; i++) {
        int ox = cx + rng_range(r, -1, 1);
        int oy = cy + rng_range(r, -1, 1);
        int oz = cz + rng_range(r, -1, 1);
        if (ox < 0 || ox >= CHUNK_SIZE_X) continue;
        if (oz < 0 || oz >= CHUNK_SIZE_Z) continue;
        if (oy < 1 || oy >= CHUNK_SIZE_Y - 1) continue;
        if (chunk_get_block(c, ox, oy, oz) == BLOCK_STONE) {
            chunk_set_block(c, ox, oy, oz, id);
        }
    }
}

void ore_sprinkle(chunk *c, unsigned seed) {
    rng r;
rng_init(&r, seed ^ ((uint64_t)c->cx * 73856093ull) ^ ((uint64_t)c->cz * 19349663ull));
int tries = 20;
for (int i = 0;
i < tries;
}
