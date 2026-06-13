#include "biome_climate.h"
#include "biome_noise.h"

#include <math.h>

// scales picked by eye. bigger divisor == smoother, broader zones. temp and
// humidity ride on independent low-freq fields so you get the full grid of
// hot/cold x wet/dry combos instead of everything correlating.
#define TEMP_SCALE   (1.0f / 380.0f)
#define HUMID_SCALE  (1.0f / 300.0f)
#define EROS_SCALE   (1.0f / 520.0f)
#define WEIRD_SCALE  (1.0f / 64.0f)

// latitude band period, in blocks. ~one full cold->hot->cold cycle.
#define LAT_PERIOD   8000.0f

static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

// remap fbm [-1,1] to [0,1]
static float to01(float n) {
    return 0.5f * (n + 1.0f);
}

float biome_climate_temperature(int wx, int wz, uint32_t seed) {
    // domain-warped so the hot/cold blobs arent boringly round
    float n = biome_warp_fbm2((float)wx * TEMP_SCALE + 11.0f,
                              (float)wz * TEMP_SCALE - 7.0f,
                              seed ^ 0xA17Eu, 0.35f, 4);
    float base = to01(n);
    // gentle latitude trend so the far north/south skew cold
    float lat = cosf((float)wz * (6.28318530f / LAT_PERIOD));
    base = base * 0.72f + (0.5f + 0.28f * lat) * 0.28f;
    return clamp01(base);
}

float biome_climate_humidity(int wx, int wz, uint32_t seed) {
    float n = biome_warp_fbm2((float)wx * HUMID_SCALE - 90.0f,
                              (float)wz * HUMID_SCALE + 40.0f,
                              seed ^ 0x5EEDu, 0.30f, 4);
    return clamp01(to01(n));
}

float biome_climate_erosion(int wx, int wz, uint32_t seed) {
    float n = biome_fbm2((float)wx * EROS_SCALE + 300.0f,
                         (float)wz * EROS_SCALE - 300.0f,
                         seed ^ 0xE205u, 3, 2.0f, 0.5f);
    return clamp01(to01(n));
}

float biome_climate_weirdness(int wx, int wz, uint32_t seed) {
    // high-freq, mostly used as a coin flip for variant biomes
    float n = biome_value2((float)wx * WEIRD_SCALE, (float)wz * WEIRD_SCALE,
                           seed ^ 0xB1A5u);
    return clamp01(to01(n));
}

void biome_climate_sample(int wx, int wz, uint32_t seed, biome_climate *out) {
    if (!out) return;
    out->temperature = biome_climate_temperature(wx, wz, seed);
    out->humidity    = biome_climate_humidity(wx, wz, seed);
    out->erosion     = biome_climate_erosion(wx, wz, seed);
    out->weirdness   = biome_climate_weirdness(wx, wz, seed);
}

void biome_climate_apply_altitude(biome_climate *c, int top_y, int sea_level) {
    if (!c) return;
    // every 24 blocks above sea drops temperature by ~0.1. clamped so we dont
    // go negative on absurd peaks. humidity also thins a little up high.
    int above = top_y - sea_level;
    if (above <= 0) return;

    float drop = (float)above / 240.0f;   // 0.1 per 24 blocks
    if (drop > 0.55f) drop = 0.55f;
    c->temperature = clamp01(c->temperature - drop);
    c->humidity    = clamp01(c->humidity - drop * 0.4f);
    // worn-down feeling at altitude, very mild
    c->erosion = clamp01(c->erosion - drop * 0.2f);
}
