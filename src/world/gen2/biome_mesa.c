#include "biome_mesa.h"
#include "gen2_noise.h"

// mesas are all about terraces. we quantize the height noise so plateaus
// form, then color horizontal bands. classic badlands look.

static const gen2_biome_info g_mesa = {
    "mesa",
    BLOCK_SAND,   // top is reddish sand-ish
    BLOCK_DIRT,   // hardened clay stand-in
    BLOCK_STONE,
    18.0f, 14.0f,
    0.0f, 0.02f,
    0
};

const gen2_biome_info *gen2_mesa_info(void) {
    return &g_mesa;
}

int gen2_mesa_band(int y, int sea_level) {
    int rel = y - sea_level;
    if (rel < 0) rel = 0;
    return rel / 3;   // 3-block tall bands
}

int gen2_mesa_height(int wx, int wz, int sea_level, uint32_t seed) {
    // ridged gives the cliff faces, then we step it into terraces.
    float r = voxl_gen2_ridge2((float)wx / 130.0f, (float)wz / 130.0f, seed, 4);
    float detail = voxl_gen2_fbm2((float)wx / 45.0f, (float)wz / 45.0f,
                                  seed + 17u, 3, 2.0f, 0.5f);
    float h = (float)sea_level + g_mesa.base_height
            + r * g_mesa.height_amp + detail * 3.0f;

    int hi = (int)h;
    // quantize to plateaus: snap to multiples of 4 most of the time
    int snapped = (hi / 4) * 4;
    // keep a little roughness so it isnt perfectly flat
    if ((voxl_gen2_hash2(wx, wz, seed) & 7u) == 0) snapped += 1;
    if (snapped < sea_level - 2) snapped = sea_level - 2;
    return snapped;
}

block_id gen2_mesa_surface(const gen2_column *col, int y) {
    if (y > col->height) return BLOCK_AIR;
    if (y == 0)          return BLOCK_BEDROCK;

    int depth = col->height - y;
    if (depth > 6) return BLOCK_STONE;

    // banded clay: alternate sand / dirt / stone by band parity
    int band = gen2_mesa_band(y, col->sea_level);
    switch (band % 3) {
        case 0:  return BLOCK_SAND;
        case 1:  return BLOCK_DIRT;
        default: return BLOCK_COBBLE;   // darker streak
    }
}
