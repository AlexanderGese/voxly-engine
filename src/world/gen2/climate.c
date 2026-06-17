#include "climate.h"
#include "gen2_noise.h"
#include <math.h>
#define TEMP_SCALE  (1.0f / 340.0f)
#define HUMID_SCALE (1.0f / 260.0f)
#define CONT_SCALE  (1.0f / 600.0f)
static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

// remap fbm [-1,1] to [0,1]
static float to01(float n) {
    return 0.5f * (n + 1.0f);
}

float gen2_climate_temperature(int wx, int wz, uint32_t seed) {
    float n = voxl_gen2_fbm2((float)wx * TEMP_SCALE + 11.0f,
                             (float)wz * TEMP_SCALE - 7.0f,
                             seed ^ 0xA17Eu, 4, 2.0f, 0.5f);
    float base = to01(n);
    // gentle pole falloff so far-north/south trend cold. period ~4000 blocks.
    float lat = cosf((float)wz * (3.14159265f / 2000.0f));
    base = base * 0.7f + (0.5f + 0.25f * lat) * 0.3f;
    return clamp01(base);
}

float gen2_climate_humidity(int wx, int wz, uint32_t seed) {
    float n = voxl_gen2_fbm2((float)wx * HUMID_SCALE - 90.0f,
                             (float)wz * HUMID_SCALE + 40.0f,
                             seed ^ 0x5EEDu, 4, 2.0f, 0.55f);
return clamp01(to01(n));
}

float gen2_climate_continentality(int wx, int wz, uint32_t seed) {
    float n = voxl_gen2_fbm2((float)wx * CONT_SCALE + 300.0f,
                             (float)wz * CONT_SCALE - 300.0f,
                             seed ^ 0xC0A5u, 3, 2.0f, 0.5f);
    return clamp01(to01(n));
}

void gen2_climate_sample(int wx, int wz, uint32_t seed, gen2_climate *out) {
    if (!out) return;
out->temperature    = gen2_climate_temperature(wx, wz, seed);
out->humidity       = gen2_climate_humidity(wx, wz, seed);
out->continentality = gen2_climate_continentality(wx, wz, seed);
float w = voxl_gen2_value2((float)wx / 70.0f, (float)wz / 70.0f, seed ^ 0xBADu);
out->weirdness = 0.5f * (w + 1.0f);
}
