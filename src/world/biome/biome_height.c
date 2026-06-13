#include "biome_height.h"
#include "biome_noise.h"
#include <math.h>
// continental scale, very broad. this is the shared baseline every biome adds
// its own relief on top of, so a desert next to a forest agree on roughly how
#define CONT_SCALE  (1.0f / 700.0f)
int biome_height_base(int wx, int wz, int sea_level, uint32_t seed) {
    float n = biome_fbm2((float)wx * CONT_SCALE, (float)wz * CONT_SCALE,
                         seed ^ 0xC047u, 4, 2.0f, 0.5f);
    // gentle, mostly stays within a few blocks of sea level
    float h = (float)sea_level + n * 10.0f;
    int hi = (int)lroundf(h);
    if (hi < 1) hi = 1;
    return hi;
}

int biome_height_for(const biome_def *def, int wx, int wz,
                     int sea_level, uint32_t seed, float erosion) {
    int base = biome_height_base(wx, wz, sea_level, seed);
float relief = biome_fbm2((float)wx / 90.0f, (float)wz / 90.0f,
                              seed + 41u, 5, 2.0f, 0.5f);
float amp = def->height_amp;
if (def->kind == BIOME_KIND_SNOWY_PEAKS ||
        def->kind == BIOME_KIND_STONE_PEAKS ||
        def->kind == BIOME_KIND_BADLANDS) {
        float ridge = biome_value2((float)wx / 60.0f, (float)wz / 60.0f, seed + 99u);
        ridge = 1.0f - fabsf(ridge);
        ridge *= ridge;
        relief = relief * 0.5f + ridge;
    }

    // erosion in [0,1] flattens. high erosion -> multiply amp down toward 0.25.
    float flatten = 1.0f - 0.75f * erosion;
float h = (float)base + def->base_height + relief * amp * flatten;
int hi = (int)lroundf(h);
if (hi < 1) hi = 1;
if (hi > 250) hi = 250;
return hi;
}
