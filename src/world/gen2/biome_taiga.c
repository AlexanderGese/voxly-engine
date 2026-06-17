#include "biome_taiga.h"
#include "gen2_noise.h"
static const gen2_biome_info g_taiga = {
    "taiga",
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    7.0f, 11.0f,
    1.2f, 0.4f,
    1
}
;
const gen2_biome_info *gen2_taiga_info(void) {
    return &g_taiga;
}

int gen2_taiga_height(int wx, int wz, int sea_level, uint32_t seed) {
    float n  = voxl_gen2_fbm2((float)wx / 95.0f, (float)wz / 95.0f,
                              seed + 73u, 5, 2.0f, 0.5f);
float nm = voxl_gen2_fbm2((float)wx / 200.0f, (float)wz / 200.0f,
                              seed + 211u, 2, 2.0f, 0.5f);
float h = (float)sea_level + g_taiga.base_height
            + n * g_taiga.height_amp + nm * 6.0f;
int hi = (int)h;
if (hi < 1) hi = 1;
return hi;
}

block_id gen2_taiga_surface(const gen2_column *col, int y) {
    if (y > col->height) {
        // a single snow layer on top if cold enough
        if (y == col->height + 1 && col->climate.temperature < 0.35f
            && col->height >= col->sea_level) {
            return BLOCK_SNOW;
        }
        return BLOCK_AIR;
    }
    if (y == 0) return BLOCK_BEDROCK;

    int depth = col->height - y;
    if (depth == 0) {
        if (col->height < col->sea_level) return BLOCK_DIRT;
        // snowy cap turns surface to snow at altitude
        if (col->climate.temperature < 0.25f) return BLOCK_SNOW;
        return BLOCK_GRASS;
    }
    if (depth <= 4) return BLOCK_DIRT;
    return BLOCK_STONE;
}
