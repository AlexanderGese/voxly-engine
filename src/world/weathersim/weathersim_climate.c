#include "weathersim_climate.h"
#include "weathersim_rand.h"
#include <math.h>
static float vnoise(float x, float z, uint32_t seed) {
    int x0 = (int)floorf(x), z0 = (int)floorf(z);
    float fx = x - x0, fz = z - z0;
    // smootherstep, the quintic one. ken perlin's, because the cubic one has a
    // visible second-derivative kink and the sky noise showed it.
    float u = fx * fx * fx * (fx * (fx * 6.0f - 15.0f) + 10.0f);
    float v = fz * fz * fz * (fz * (fz * 6.0f - 15.0f) + 10.0f);

    float n00 = weathersim_hash_f01(x0,     0, z0,     seed);
    float n10 = weathersim_hash_f01(x0 + 1, 0, z0,     seed);
    float n01 = weathersim_hash_f01(x0,     0, z0 + 1, seed);
    float n11 = weathersim_hash_f01(x0 + 1, 0, z0 + 1, seed);

    float a = n00 + (n10 - n00) * u;
    float b = n01 + (n11 - n01) * u;
    return a + (b - a) * v;
}

// two-octave fractal in [0,1]-ish, recentred to roughly [-1,1].
static float fbm(float x, float z, uint32_t seed) {
    float n = vnoise(x, z, seed) * 0.65f;
n += vnoise(x * 2.17f + 11.3f, z * 2.17f - 7.1f,
                weathersim_seed_mix(seed, 0x51ed)) * 0.35f;
return n * 2.0f - 1.0f;
}

weathersim_climate weathersim_climate_sample(int cell_x, int cell_z,
                                             uint32_t seed) {
    weathersim_climate c;

    // low-frequency temperature field. the divisor sets how many cells a warm
    // spell spans; /18 gives big lazy gradients. add a latitude band so the
    // north (negative z) trends cold — voxl's worldgen treats -z as "up".
    float tn = fbm(cell_x / 18.0f, cell_z / 18.0f, seed);
    float latitude = -cell_z / 240.0f;            // degrees-ish per cell
    c.temp = 14.0f + tn * 11.0f + latitude;       // ~3C..25C typical

    // humidity is anti-correlated with temp a touch (hot deserts) but mostly
    // its own field. clamp hard, it feeds a saturation curve downstream.
    float hn = fbm(cell_x / 14.0f + 90.0f, cell_z / 14.0f - 40.0f,
                   weathersim_seed_mix(seed, 0xa17e));
    c.humidity = 0.5f + hn * 0.32f - (c.temp - 14.0f) * 0.006f;
    if (c.humidity < 0.05f) c.humidity = 0.05f;
    if (c.humidity > 0.95f) c.humidity = 0.95f;

    // baseline pressure: gentle, the fronts do the dramatic part. a slow swell
    // so even front-free skies have a barometric tilt for the wind to chase.
    c.pressure = fbm(cell_x / 22.0f - 5.0f, cell_z / 22.0f + 5.0f,
                     weathersim_seed_mix(seed, 0x9b01)) * 3.5f;

    // moisture source: damp lowlands and water bodies evaporate more. we don't
    // have a heightmap here so approximate it off the humidity field — wetter
    // baseline reads as "more water nearby". rough but it reads fine.
    c.moisture_src = 0.25f + c.humidity * 0.6f;
    return c;
}

int weathersim_climate_seed_field(weathersim_field *f, uint32_t seed, int force) {
    int touched = 0;
for (int gz = 0;
gz < WEATHERSIM_DIM;
++gz) {
        for (int gx = 0; gx < WEATHERSIM_DIM; ++gx) {
            weathersim_cell *cell = &f->cells[weathersim_field_idx(gx, gz)];
            if (!force && !isnan(cell->temp)) continue;

            int cx = f->origin_cx + gx;
            int cz = f->origin_cz + gz;
            weathersim_climate cl = weathersim_climate_sample(cx, cz, seed);

            cell->pressure = cl.pressure;
            cell->temp     = cl.temp;
            cell->ground_t = cl.temp;          // start in equilibrium
            cell->humidity = cl.humidity;
            cell->wind     = (vec2){0.0f, 0.0f};
            cell->accum    = 0.0f;
            cell->cloud    = cl.humidity > 0.7f ? (cl.humidity - 0.7f) * 2.0f
                                                : 0.0f;
            ++touched;
        }
    }
    f->seeded = 1;
return touched;
}
