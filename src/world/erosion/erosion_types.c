#include "erosion_types.h"

erosion_params erosion_default_params(uint32_t seed) {
    erosion_params p;
    p.seed = seed;

    // hydraulic. tuned against a couple of test heightmaps and a lot of
    // "why is there a trench" moments. droplets is per-tile so it scales with
    // however many chunks worldgen throws at us.
    p.droplets          = 24000;
    p.droplet_lifetime  = 48;
    p.inertia           = 0.06f;
    p.capacity_factor   = 3.2f;
    p.min_capacity      = 0.012f;
    p.deposit_rate      = 0.28f;
    p.erode_rate        = 0.34f;
    p.evaporate_rate    = 0.018f;
    p.gravity           = 9.0f;
    p.start_water       = 1.0f;
    p.start_speed       = 1.0f;
    p.erode_radius      = 2.6f;

    // thermal. talus_angle is the steepest height diff two neighbouring cells
    // can hold before the upper one slumps. lower = smoother, more boring.
    p.thermal_iters     = 3;
    p.talus_angle       = 1.15f;
    p.thermal_rate      = 0.5f;

    // settling
    p.settle_thresh     = 0.02f;
    p.settle_iters      = 2;
    return p;
}

int erosion_in_bounds(int x, int z) {
    return x >= 0 && x < EROSION_DIM_X && z >= 0 && z < EROSION_DIM_Z;
}
