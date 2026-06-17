#include "biome_registry.h"
#include "climate.h"
#include "biome_mesa.h"
#include "biome_swamp.h"
#include "biome_taiga.h"
#include "biome_savanna.h"
#include "biome_jungle.h"

const gen2_biome_info *gen2_registry_info(gen2_biome_id id) {
    switch (id) {
        case GEN2_MESA:    return gen2_mesa_info();
        case GEN2_SWAMP:   return gen2_swamp_info();
        case GEN2_TAIGA:   return gen2_taiga_info();
        case GEN2_SAVANNA: return gen2_savanna_info();
        case GEN2_JUNGLE:  return gen2_jungle_info();
        default:           return gen2_savanna_info();
    }
}

gen2_biome_id gen2_registry_pick(const gen2_climate *c) {
    float t = c->temperature;
    float h = c->humidity;

    // cold wins first regardless of humidity
    if (t < 0.30f) return GEN2_TAIGA;

    if (t > 0.66f) {
        // hot band split by humidity
        if (h < 0.30f) return GEN2_MESA;     // hot + dry
        if (h > 0.66f) return GEN2_JUNGLE;   // hot + wet
        return GEN2_SAVANNA;                 // hot + mid
    }

    // temperate band
    if (h > 0.62f) return GEN2_SWAMP;        // warm + wet
    return GEN2_SAVANNA;                     // default dry grassland
}

int gen2_registry_height(gen2_biome_id id, int wx, int wz,
                         int sea_level, uint32_t seed) {
    switch (id) {
        case GEN2_MESA:    return gen2_mesa_height(wx, wz, sea_level, seed);
        case GEN2_SWAMP:   return gen2_swamp_height(wx, wz, sea_level, seed);
        case GEN2_TAIGA:   return gen2_taiga_height(wx, wz, sea_level, seed);
        case GEN2_SAVANNA: return gen2_savanna_height(wx, wz, sea_level, seed);
        case GEN2_JUNGLE:  return gen2_jungle_height(wx, wz, sea_level, seed);
        default:           return sea_level;
    }
}

block_id gen2_registry_surface(const gen2_column *col, int y) {
    switch (col->biome) {
        case GEN2_MESA:    return gen2_mesa_surface(col, y);
        case GEN2_SWAMP:   return gen2_swamp_surface(col, y);
        case GEN2_TAIGA:   return gen2_taiga_surface(col, y);
        case GEN2_SAVANNA: return gen2_savanna_surface(col, y);
        case GEN2_JUNGLE:  return gen2_jungle_surface(col, y);
        default:           return BLOCK_AIR;
    }
}

void gen2_registry_build_column(int wx, int wz, int sea_level,
                                uint32_t seed, gen2_column *out) {
    if (!out) return;
    out->wx = wx;
    out->wz = wz;
    out->sea_level = sea_level;
    out->seed = seed;
    gen2_climate_sample(wx, wz, seed, &out->climate);
    out->biome  = gen2_registry_pick(&out->climate);
    out->height = gen2_registry_height(out->biome, wx, wz, sea_level, seed);
}
