#include "decoration.h"
#include "gen2_noise.h"
#include "biome_registry.h"

// per-column roll. we bias kinds by biome flavor so jungles get bushes,
// swamps get mushrooms, savanna gets dead bushes, etc.

gen2_deco_kind gen2_deco_pick(const gen2_column *col) {
    if (col->height < col->sea_level) return GEN2_DECO_NONE;   // underwater

    const gen2_biome_info *bi = gen2_registry_info(col->biome);
    float roll = voxl_gen2_hash_f01(col->wx, col->wz, col->seed ^ 0xDEC0u);

    // grass density gates the common tuft
    if (roll < bi->grass_density * 0.5f) {
        // pick a flavor by a second hash
        uint32_t k = voxl_gen2_hash2(col->wx, col->wz, col->seed ^ 0xF10Au);
        switch (col->biome) {
            case GEN2_SWAMP:
                return (k & 3u) == 0 ? GEN2_DECO_MUSHROOM : GEN2_DECO_GRASS;
            case GEN2_JUNGLE:
                return (k & 1u) ? GEN2_DECO_BUSH : GEN2_DECO_GRASS;
            case GEN2_MESA:
            case GEN2_SAVANNA:
                return (k & 7u) == 0 ? GEN2_DECO_DEADBUSH : GEN2_DECO_GRASS;
            default:
                return GEN2_DECO_GRASS;
        }
    }
    return GEN2_DECO_NONE;
}

int gen2_deco_place(gen2_place_buf *out, const gen2_column *col) {
    gen2_deco_kind k = gen2_deco_pick(col);
    if (k == GEN2_DECO_NONE) return 0;

    int x = col->wx, z = col->wz, y = col->height + 1;

    switch (k) {
        case GEN2_DECO_BUSH:
            // a tiny leaf nub
            return gen2_place_add(out, x, y, z, BLOCK_LEAVES);
        case GEN2_DECO_MUSHROOM:
            // torch as a glowing toadstool stand-in
            return gen2_place_add(out, x, y, z, BLOCK_TORCH);
        case GEN2_DECO_DEADBUSH:
            // no proper sprite, use a single leaf as twig
            return gen2_place_add(out, x, y, z, BLOCK_LEAVES);
        case GEN2_DECO_GRASS:
        default:
            // we lack a grass-blade block, so this is a no-op marker column.
            // returning 0 keeps the buffer clean but the pick is still useful
            // metadata for whoever drives rendering of cross-quads later.
            return 0;
    }
}
