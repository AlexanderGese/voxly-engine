#include "oregen_field.h"
#include "oregen_table.h"
#include "oregen_curve.h"
#include "oregen_noise.h"
#include "oregen_rand.h"

// the mask noise frequency. low so deposits are several blocks across, not
// per-voxel salt.
#define FIELD_FREQ 0.18f

float oregen_field_density(const oregen_ore *ore, int x, int y, int z,
                           uint32_t seed) {
    if (!ore) return 0.0f;

    float band = oregen_curve_weight(ore, y);
    if (band <= 0.0f) return 0.0f;       // out of band, no point sampling

    // per-ore seed so kinds dont share a mask (otherwise gold and iron would
    // place in the exact same blobs).
    uint32_t os = oregen_seed_mix(seed, (uint32_t)(ore->block + 1) * 0x632be59bu);

    // fbm in [-1,1] -> remap to [0,1], then sharpen so the deposits have
    // crisp-ish cores and thin tails.
    float n = oregen_noise_fbm3((float)x * FIELD_FREQ,
                                (float)y * FIELD_FREQ * (1.0f / ore->squish),
                                (float)z * FIELD_FREQ,
                                os, 3, 2.0f, 0.5f);
    float m = (n + 1.0f) * 0.5f;
    m = m * m;                            // sharpen

    return band * m;
}

int oregen_field_hit(const oregen_ore *ore, int x, int y, int z,
                     float threshold, uint32_t seed) {
    return oregen_field_density(ore, x, y, z, seed) >= threshold;
}

int oregen_field_pick(int x, int y, int z, float threshold, uint32_t seed) {
    int best = -1;
    float best_d = threshold;            // must beat the threshold to count

    int kinds = oregen_table_count();
    for (int k = 0; k < kinds; k++) {
        const oregen_ore *ore = oregen_table_at(k);
        if (!oregen_curve_in_band(y, ore->y_min, ore->y_max)) continue;

        float d = oregen_field_density(ore, x, y, z, seed);
        if (d > best_d) {
            best_d = d;
            best = k;
        }
    }
    return best;
}

float oregen_field_estimate_rate(const oregen_ore *ore, float threshold,
                                 int samples, uint32_t seed) {
    if (!ore || samples <= 0) return 0.0f;

    oregen_rng rr;
    oregen_rng_seed(&rr, seed ^ 0x1ce4e5b9u);

    int hits = 0;
    for (int i = 0; i < samples; i++) {
        // random voxel within the band and a generous horizontal spread.
        int x = oregen_rng_range(&rr, -512, 512);
        int z = oregen_rng_range(&rr, -512, 512);
        int y = oregen_rng_range(&rr, ore->y_min, ore->y_max);
        if (oregen_field_hit(ore, x, y, z, threshold, seed)) hits++;
    }
    return (float)hits / (float)samples;
}
