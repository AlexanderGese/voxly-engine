#include "treegen_grass.h"
#include "treegen_rand.h"
#include "../../config.h"

// ground scatter. one block per cell, decided by a couple of hashes so it's
// stable and seam-consistent. no allocation, no state; pure functions over
// (coords, seed). that's deliberate: terrain can call this from any thread.

treegen_scatter treegen_scatter_default(void) {
    treegen_scatter s;
    s.density   = 90;    // ~35% of eligible cells, after the kind roll thins it
    s.w_grass   = 80;
    s.w_flower  = 14;
    s.w_deadbush = 6;
    return s;
}

// only put cover on ground the player would expect it on.
static int surface_ok(block_id b) {
    return b == BLOCK_GRASS || b == BLOCK_DIRT || b == BLOCK_SAND;
}

treegen_cover treegen_grass_pick(int wx, int wz, block_id surface_block,
                                 const treegen_scatter *cfg, uint32_t seed) {
    if (!surface_ok(surface_block)) return TREEGEN_COVER_NONE;

    // first gate: does anything grow here at all.
    uint32_t h = treegen_hash2(wx, wz, seed ^ 0x9165a73fu);
    if ((h & 0xff) >= cfg->density) return TREEGEN_COVER_NONE;

    // sand strongly prefers deadbush; grass blocks prefer real grass. nudge the
    // weights by substrate so deserts and meadows feel different for free.
    int wg = cfg->w_grass, wf = cfg->w_flower, wd = cfg->w_deadbush;
    if (surface_block == BLOCK_SAND) { wg = 4; wf = 2; wd = 40; }
    else if (surface_block == BLOCK_DIRT) { wf /= 2; }

    int total = wg + wf + wd;
    if (total <= 0) return TREEGEN_COVER_NONE;

    // second hash chooses the kind so it's independent of the gate above.
    uint32_t k = treegen_hash2(wx, wz, seed ^ 0x2c1b3a55u) % (uint32_t)total;
    if ((int)k < wg) return TREEGEN_COVER_GRASS;
    if ((int)k < wg + wf) return TREEGEN_COVER_FLOWER;
    return TREEGEN_COVER_DEADBUSH;
}

block_id treegen_cover_block(treegen_cover c, int wx, int wz, uint32_t seed) {
    switch (c) {
    case TREEGEN_COVER_GRASS:
        // no dedicated tall-grass block in the enum yet, so we reuse leaves as
        // the stand-in tuft. (when the atlas gets a foliage tile this is the one
        // line that changes.)
        return BLOCK_LEAVES;
    case TREEGEN_COVER_FLOWER:
        // pick one of a couple of "flower" stand-ins by a tint hash. brick/snow
        // read as red/white accents until proper flower tiles land.
        return (treegen_hash2(wx, wz, seed ^ 0x51edu) & 1) ? BLOCK_BRICK : BLOCK_SNOW;
    case TREEGEN_COVER_DEADBUSH:
        return BLOCK_WOOD;   // a lone wood stub reads as a dry twig
    default:
        return BLOCK_AIR;
    }
}

int treegen_grass_scatter_chunk(int chunk_wx, int chunk_wz,
                                const block_id *surf, treegen_cover *out,
                                const treegen_scatter *cfg, uint32_t seed) {
    int placed = 0;
    for (int z = 0; z < CHUNK_SIZE_Z; z++) {
        for (int x = 0; x < CHUNK_SIZE_X; x++) {
            int idx = z * CHUNK_SIZE_X + x;
            treegen_cover c = treegen_grass_pick(chunk_wx + x, chunk_wz + z,
                                                 surf[idx], cfg, seed);
            out[idx] = c;
            if (c != TREEGEN_COVER_NONE) placed++;
        }
    }
    return placed;
}
