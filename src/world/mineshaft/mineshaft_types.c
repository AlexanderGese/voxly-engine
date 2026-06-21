#include "mineshaft_types.h"
#include "../block_ext.h"

mineshaft_config mineshaft_config_default(void) {
    mineshaft_config c;
    c.region_size  = 12;     // chunks per region, shafts are rare
    c.spawn_chance = 0.55f;
    c.depth_min    = 6;      // never dig below this floor_y (bedrock buffer)
    c.depth_below  = 14;     // top floor sits this far under the surface
    c.max_pieces   = 120;    // corridors + rooms in one maze
    c.corridor_len = 5;      // blocks per corridor cell (the grid pitch)
    c.max_voxels   = 60000;  // bound a single shaft so it can't eat the budget

    // palette. there's no dedicated cobweb tile so we borrow tall grass: it's
    // plant-class (non-solid, transparent, cross-meshed) which reads as stringy
    // junk hanging in a corner. swap mat_web the day a real web tile lands.
    c.mat_support = BLOCK_FENCE;
    c.mat_beam    = BLOCK_WOOD;
    c.mat_fill    = BLOCK_PLANKS;
    c.mat_rail    = BLOCK_SLAB_WOOD;
    c.mat_web     = BLOCK_TALL_GRASS;
    c.mat_torch   = BLOCK_TORCH;
    c.mat_chest   = BLOCK_CHEST_BLOCK;
    c.ore_common  = BLOCK_COAL_ORE;
    c.ore_rare    = BLOCK_IRON_ORE;
    return c;
}
