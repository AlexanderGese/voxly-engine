#include "cavegen_types.h"

#include "../../config.h"
#include <math.h>

cavegen_params cavegen_default_params(uint32_t seed) {
    cavegen_params p;
    p.seed = seed;

    // these numbers are the result of a lot of staring at f3 and going "no".
    p.fill_chance     = 0.47f;
    p.ca_iterations   = 4;
    p.ca_birth        = 13;   // out of 26 neighbours
    p.ca_survive      = 11;

    p.cheese_scale    = 0.045f;
    p.cheese_thresh   = -0.18f;

    p.worm_count      = 3;
    p.worm_max_steps  = 220;
    p.worm_radius     = 2.4f;
    p.worm_radius_var = 1.1f;
    p.worm_step       = 1.0f;
    p.worm_pitch_max  = 0.28f;

    p.min_y           = 6;
    p.surface_margin  = 5;
    p.water_table     = WORLD_SEA_LEVEL - 6;

    p.min_region_cells = 24;
    return p;
}
