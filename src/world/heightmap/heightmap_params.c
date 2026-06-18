#include "heightmap_params.h"

heightmap_params heightmap_default_params(uint32_t seed, int sea_level) {
    heightmap_params p;
    p.seed = seed;
    p.sea_level = sea_level;

    // frequencies. continents are huge, hills are not. these are world units
    // per noise cell, so a 1400 cont_scale means a full noise wavelength is
    // ~1400 blocks, roughly a continent.
    p.cont_scale  = 1400.0f;
    p.eros_scale  =  520.0f;
    p.pv_scale    =  180.0f;
    p.weird_scale =  240.0f;

    p.cont_octaves = 5;
    p.eros_octaves = 4;
    p.pv_octaves   = 4;

    p.cont_warp = 0.35f;

    // vertical reach. the continent spline alone can swing ~70 blocks between
    // deep ocean and inland plateau; peaks/valleys adds up to ~90 on top but
    // only where erosion lets it.
    p.cont_amp = 72.0f;
    p.pv_amp   = 90.0f;

    p.base_height = sea_level;

    p.min_height = 2;
    p.max_height = 250;   // keep headroom under CHUNK_SIZE_Y

    p.erosion_bias = 0.85f;
    return p;
}
