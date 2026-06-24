#include "rivers_types.h"

#include "../../config.h"

rivers_params rivers_default_params(uint32_t seed, int sea_level) {
    rivers_params p;
    p.seed      = seed;
    p.sea_level = sea_level;

    // these are the numbers that stopped giving me rivers running uphill. mostly.
    p.rain_per_cell   = 1.0f;
    p.rain_jitter     = 0.35f;
    p.river_threshold = 48;     // ~half a chunk worth of catchment

    p.max_depth       = 4;
    p.depth_per_log   = 0.55f;
    p.bank_width      = 1;

    p.fill_max_y      = sea_level + 18;
    p.min_lake_cells  = 6;
    p.carve_outlet    = 1;

    p.max_trace_steps = RIVERS_CELLS * 2; // way more than any real path needs
    p.min_source_y    = sea_level - 2;
    return p;
}

rivers_dir rivers_dir_opposite(rivers_dir d) {
    if (d == RIVERS_DIR_NONE) return RIVERS_DIR_NONE;
    // the 8 compass dirs are laid out so opposite is +4 mod 8.
    return (rivers_dir)((d + 4) & 7);
}
