#include "ravine_types.h"
ravine_params ravine_default_params(uint32_t seed, int sea_level) {
    ravine_params p;
    p.seed      = seed;
    p.sea_level = sea_level;

    p.rarity      = 22;     // ~1 region in 22 seeds a ravine
    p.min_floor_y = 8;

    p.max_depth    = 30;
    p.half_width   = 4.5f;
    p.wall_slope   = 0.55f;   // ~half a block of run per block of rise: steep
    p.width_wobble = 0.35f;

    p.knot_span   = 9;        // cells between control knots
    p.knot_jitter = 5.0f;
    p.drift       = 1.2f;

    p.wall_jitter   = 1.5f;
    p.strata_jitter = 1;
    p.expose_ores   = 0;
    return p;
}

int ravine_clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
if (v > hi) return hi;
return v;
}

int ravine_cell_index(int cx, int cz) {
    if (cx < 0 || cx >= RAVINE_DIM_X || cz < 0 || cz >= RAVINE_DIM_Z)
        return -1;
    return cx + cz * RAVINE_DIM_X;
}
