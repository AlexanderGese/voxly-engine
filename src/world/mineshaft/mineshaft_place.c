#include "mineshaft_place.h"
#include "mineshaft_rand.h"
#include "../../config.h"

// floor-divide rounding toward negative infinity so regions tile cleanly across
// the origin instead of mirroring around it.
static int floordiv(int a, int b) {
    int q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0))) q--;
    return q;
}

int mineshaft_pick(const mineshaft_config *cfg, int ccx, int ccz,
                   uint32_t world_seed, mineshaft_site *out) {
    int rsize = cfg->region_size > 0 ? cfg->region_size : 12;
    int rx = floordiv(ccx, rsize);
    int rz = floordiv(ccz, rsize);

    // one roll per region. miss -> no shaft here.
    uint32_t rseed = mineshaft_seed_mix(world_seed ^ 0x4d494e45u,   // 'MINE'
                                        mineshaft_hash2(rx, rz, world_seed));
    mineshaft_rng rng;
    mineshaft_rng_seed(&rng, rseed);
    if (!mineshaft_rng_chance(&rng, cfg->spawn_chance)) return 0;

    // jitter the anchor inside the region so shafts don't snap to a lattice,
    // keeping a margin off the edges so the grid stays mostly in one region.
    int region_blocks = rsize * CHUNK_SIZE_X;
    int margin = region_blocks / 5;
    int ox = mineshaft_rng_range(&rng, margin, region_blocks - margin - 1);
    int oz = mineshaft_rng_range(&rng, margin, region_blocks - margin - 1);

    out->anchor_x = rx * region_blocks + ox;
    out->anchor_z = rz * region_blocks + oz;
    out->floor_y  = WORLD_SEA_LEVEL;   // placeholder; driver resolves from height
    out->seed     = mineshaft_seed_mix(rseed, 0x9e3779b9u);
    return 1;
}
