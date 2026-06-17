#include "biome_swamp.h"
#include "gen2_noise.h"

// swamps hug sea level. tiny amplitude, lots of pools. we keep the top
// within a block or two of the waterline so it reads soggy.

static const gen2_biome_info g_swamp = {
    "swamp",
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    -1.0f, 2.0f,
    0.35f, 0.7f,
    0
};

const gen2_biome_info *gen2_swamp_info(void) {
    return &g_swamp;
}

int gen2_swamp_is_pool(int wx, int wz, int sea_level, uint32_t seed) {
    (void)sea_level;
    // low-freq blobby field; below a threshold == depression == pool
    float n = voxl_gen2_fbm2((float)wx / 38.0f, (float)wz / 38.0f,
                             seed ^ 0x9000u, 3, 2.0f, 0.5f);
    return n < -0.12f;
}

int gen2_swamp_height(int wx, int wz, int sea_level, uint32_t seed) {
    float n = voxl_gen2_fbm2((float)wx / 60.0f, (float)wz / 60.0f,
                             seed + 51u, 3, 2.0f, 0.45f);
    float h = (float)sea_level + g_swamp.base_height + n * g_swamp.height_amp;
    int hi = (int)h;
    if (gen2_swamp_is_pool(wx, wz, sea_level, seed)) {
        // carve a shallow dip so water can sit on top
        hi -= 2;
    }
    if (hi < 1) hi = 1;
    return hi;
}

block_id gen2_swamp_surface(const gen2_column *col, int y) {
    if (y > col->height) {
        // fill the gap up to sea level with water (pools)
        if (y <= col->sea_level) return BLOCK_WATER;
        return BLOCK_AIR;
    }
    if (y == 0) return BLOCK_BEDROCK;

    int depth = col->height - y;
    if (depth == 0) {
        // muddy top: grass normally, dirt if submerged
        return (col->height < col->sea_level) ? BLOCK_DIRT : BLOCK_GRASS;
    }
    if (depth <= 3) return BLOCK_DIRT;
    return BLOCK_STONE;
}
