#include "tree_dispatch.h"
#include "gen2_noise.h"
#include "biome_registry.h"
#include "tree_oak.h"
#include "tree_pine.h"
#include "tree_palm.h"

gen2_tree_species gen2_tree_species_for(gen2_biome_id biome) {
    switch (biome) {
        case GEN2_TAIGA:   return GEN2_TREE_PINE;
        case GEN2_JUNGLE:  return GEN2_TREE_OAK;   // big leafy stand-in
        case GEN2_SWAMP:   return GEN2_TREE_OAK;
        case GEN2_SAVANNA: return GEN2_TREE_PALM;  // acacia-ish
        case GEN2_MESA:    return GEN2_TREE_NONE;  // basically barren
        default:           return GEN2_TREE_NONE;
    }
}

int gen2_tree_should_grow(const gen2_column *col) {
    if (col->height < col->sea_level) return 0;   // no trees underwater
    const gen2_biome_info *bi = gen2_registry_info(col->biome);
    if (bi->tree_density <= 0.0f) return 0;

    // tree_density can exceed 1 (dense forests). we model that as: a base
    // chance per column, scaled, capped. one roll per column.
    float chance = bi->tree_density * 0.06f;   // 0.06 per "density unit"
    if (chance > 0.85f) chance = 0.85f;

    float roll = voxl_gen2_hash_f01(col->wx, col->wz, col->seed ^ 0x77EEu);
    return roll < chance;
}

int gen2_tree_grow(gen2_place_buf *out, const gen2_column *col) {
    if (!gen2_tree_should_grow(col)) return 0;

    gen2_tree_species sp = gen2_tree_species_for(col->biome);
    int bx = col->wx, by = col->height + 1, bz = col->wz;
    // per-tree seed so neighbours dont grow identical trees
    uint32_t ts = voxl_gen2_hash2(col->wx, col->wz, col->seed ^ 0x7EE5u);

    switch (sp) {
        case GEN2_TREE_OAK:  return gen2_tree_oak_build(out, bx, by, bz, ts);
        case GEN2_TREE_PINE: return gen2_tree_pine_build(out, bx, by, bz, ts);
        case GEN2_TREE_PALM: return gen2_tree_palm_build(out, bx, by, bz, ts);
        default:             return 0;
    }
}
