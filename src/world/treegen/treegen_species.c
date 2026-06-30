#include "treegen_types.h"

// per-species default tunables. this is just a data table; tweaking a tree
// shape should never need a code change beyond here (famous last words).

treegen_species treegen_species_get(treegen_kind kind) {
    treegen_species s;
    s.kind = kind;
    s.pal.wood = BLOCK_WOOD;
    s.pal.leaf = BLOCK_LEAVES;
    s.angle_jitter = 6.0f;
    s.leaf_density = 70;

    switch (kind) {
    case TREEGEN_OAK:
        s.iterations   = 3;
        s.seg_len      = 2;
        s.yaw_deg      = 25.0f;
        s.pitch_deg    = 22.0f;
        s.roll_deg     = 90.0f;
        s.angle_jitter = 9.0f;
        s.trunk_radius = 1;
        s.leaf_radius  = 2;
        s.leaf_density = 72;
        s.min_height   = 4;
        s.max_height   = 18;
        break;
    case TREEGEN_PINE:
        s.iterations   = 3;
        s.seg_len      = 1;
        s.yaw_deg      = 30.0f;
        s.pitch_deg    = 38.0f;   // whorls fan out hard
        s.roll_deg     = 60.0f;
        s.angle_jitter = 4.0f;
        s.trunk_radius = 1;
        s.leaf_radius  = 2;
        s.leaf_density = 80;
        s.min_height   = 6;
        s.max_height   = 26;
        break;
    case TREEGEN_BIRCH:
        s.iterations   = 4;
        s.seg_len      = 2;
        s.yaw_deg      = 18.0f;
        s.pitch_deg    = 14.0f;
        s.roll_deg     = 100.0f;
        s.angle_jitter = 7.0f;
        s.trunk_radius = 0;       // skinny, single-block trunk
        s.leaf_radius  = 1;
        s.leaf_density = 58;
        s.min_height   = 6;
        s.max_height   = 20;
        break;
    case TREEGEN_PALM:
        s.iterations   = 3;
        s.seg_len      = 1;
        s.yaw_deg      = 40.0f;
        s.pitch_deg    = 50.0f;
        s.roll_deg     = 45.0f;
        s.angle_jitter = 5.0f;
        s.trunk_radius = 0;
        s.leaf_radius  = 1;
        s.leaf_density = 90;
        s.min_height   = 5;
        s.max_height   = 14;
        break;
    case TREEGEN_BUSH:
        s.iterations   = 2;
        s.seg_len      = 1;
        s.yaw_deg      = 35.0f;
        s.pitch_deg    = 30.0f;
        s.roll_deg     = 70.0f;
        s.angle_jitter = 12.0f;
        s.trunk_radius = 0;
        s.leaf_radius  = 1;
        s.leaf_density = 65;
        s.min_height   = 1;
        s.max_height   = 4;
        break;
    default:
        // TREEGEN_NONE / out of range. return a harmless dud so callers that
        // forget to check kind dont splat a tree of garbage.
        s.kind         = TREEGEN_NONE;
        s.iterations   = 0;
        s.seg_len      = 1;
        s.yaw_deg      = 0.0f;
        s.pitch_deg    = 0.0f;
        s.roll_deg     = 0.0f;
        s.trunk_radius = 0;
        s.leaf_radius  = 0;
        s.min_height   = 0;
        s.max_height   = 0;
        break;
    }
    return s;
}
