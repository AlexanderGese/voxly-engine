#include "biome_jungle.h"
#include "gen2_noise.h"

// hot wet and bumpy. high tree density, deep dirt for big roots.

static const gen2_biome_info g_jungle = {
    "jungle",
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    9.0f, 16.0f,
    2.5f, 1.0f,
    0
};

const gen2_biome_info *gen2_jungle_info(void) {
    return &g_jungle;
}

int gen2_jungle_height(int wx, int wz, int sea_level, uint32_t seed) {
    // two fbm layers + a high-freq bump for the lumpy floor
    float n  = voxl_gen2_fbm2((float)wx / 80.0f, (float)wz / 80.0f,
                              seed + 303u, 5, 2.0f, 0.5f);
    float bump = voxl_gen2_fbm2((float)wx / 18.0f, (float)wz / 18.0f,
                                seed + 404u, 2, 2.0f, 0.5f);
    float h = (float)sea_level + g_jungle.base_height
            + n * g_jungle.height_amp + bump * 4.0f;
    int hi = (int)h;
    if (hi < 1) hi = 1;
    return hi;
}

block_id gen2_jungle_surface(const gen2_column *col, int y) {
    if (y > col->height) return BLOCK_AIR;
    if (y == 0) return BLOCK_BEDROCK;

    int depth = col->height - y;
    if (depth == 0) {
        return (col->height < col->sea_level) ? BLOCK_DIRT : BLOCK_GRASS;
    }
    if (depth <= 5) return BLOCK_DIRT;   // deep dirt for roots
    return BLOCK_STONE;
}
