#include "surface_blend.h"
#include "biome_registry.h"
#include "climate.h"

#include <math.h>

// we sample the picked biome at each neighbor and blend its height. this
// is the cheap "sample around me and average" trick. not physically right
// but it kills the cliff seams between biomes.

int gen2_blend_height(int wx, int wz, int sea_level, uint32_t seed, int radius) {
    if (radius < 0) radius = 0;
    if (radius > 4) radius = 4;

    float acc = 0.0f, wsum = 0.0f;
    for (int dz = -radius; dz <= radius; dz++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int sx = wx + dx;
            int sz = wz + dz;
            gen2_climate c;
            gen2_climate_sample(sx, sz, seed, &c);
            gen2_biome_id b = gen2_registry_pick(&c);
            int hh = gen2_registry_height(b, sx, sz, sea_level, seed);
            // inverse-distance-ish weight, center heaviest
            float w = 1.0f / (1.0f + (float)(dx * dx + dz * dz));
            acc  += (float)hh * w;
            wsum += w;
        }
    }
    if (wsum <= 0.0f) return sea_level;
    return (int)lroundf(acc / wsum);
}

float gen2_blend_edge_factor(int wx, int wz, int sea_level, uint32_t seed) {
    (void)sea_level;
    gen2_climate c0;
    gen2_climate_sample(wx, wz, seed, &c0);
    gen2_biome_id home = gen2_registry_pick(&c0);

    int different = 0, total = 0;
    const int r = 3;
    for (int dz = -r; dz <= r; dz += 3) {
        for (int dx = -r; dx <= r; dx += 3) {
            if (dx == 0 && dz == 0) continue;
            gen2_climate c;
            gen2_climate_sample(wx + dx, wz + dz, seed, &c);
            if (gen2_registry_pick(&c) != home) different++;
            total++;
        }
    }
    if (total == 0) return 0.0f;
    return (float)different / (float)total;
}
