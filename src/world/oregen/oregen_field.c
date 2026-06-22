#include "oregen_field.h"
#include "oregen_table.h"
#include "oregen_curve.h"
#include "oregen_noise.h"
#include "oregen_rand.h"
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
oregen_rng rr;
oregen_rng_seed(&rr, seed ^ 0x1ce4e5b9u);
int hits = 0;
for (int i = 0;
i < samples;
}
