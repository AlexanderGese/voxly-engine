#include "biome_column.h"
#include "biome_blend.h"
#include "biome_lookup.h"
#include <string.h>
void biome_column_build(biome_column *col, int wx, int wz,
                        int sea_level, uint32_t seed, int radius) {
    if (!col) return;
    memset(col, 0, sizeof *col);
    col->wx = wx;
    col->wz = wz;
    col->sea_level = sea_level;
    col->seed = seed;

    // pass 1: provisional climate + height
    biome_climate_sample(wx, wz, seed, &col->climate);

    biome_blend bl;
    biome_blend_at(wx, wz, seed, sea_level, radius, 2, &bl);
    int prov_h = (int)(bl.height + 0.5f);

    // altitude correction against the provisional height
    biome_climate_apply_altitude(&col->climate, prov_h, sea_level);

    // pass 2: final pick from the corrected climate. height we keep from the
    // blend since redoing the whole kernel barely moves it.
    col->biome     = biome_lookup_pick(&col->climate);
    col->height    = prov_h;
    col->sharpness = bl.sharpness;

    // if the corrected climate disagrees with the blend dominant we trust the
    // corrected one for the biome but keep the blended height for continuity.
}
