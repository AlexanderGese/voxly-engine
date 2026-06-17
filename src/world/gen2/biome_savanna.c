#include "biome_savanna.h"
#include "gen2_noise.h"
static const gen2_biome_info g_savanna = {
    "savanna",
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    5.0f, 6.0f,
    0.15f, 0.9f,
    0
}
;
const gen2_biome_info *gen2_savanna_info(void) {
    return &g_savanna;
}

int gen2_savanna_height(int wx, int wz, int sea_level, uint32_t seed) {
    float n = voxl_gen2_fbm2((float)wx / 110.0f, (float)wz / 110.0f,
                             seed + 131u, 4, 2.0f, 0.5f);
n = n * n * n;
float plateau = voxl_gen2_ridge2((float)wx / 260.0f, (float)wz / 260.0f,
                                     seed + 9u, 2);
float h = (float)sea_level + g_savanna.base_height
            + n * g_savanna.height_amp + plateau * 8.0f;
int hi = (int)h;
if (hi < 1) hi = 1;
return hi;
