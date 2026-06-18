#ifndef WORLD_HEIGHTMAP_PARAMS_H
#define WORLD_HEIGHTMAP_PARAMS_H

#include <stdint.h>

// the knobs for the spline terrain. one of these is built per world from the
// seed and reused for every column, so it has to be plain data with no
// pointers into the noise tables. defaults live in heightmap_default_params().
//
// the three driving fields are scaled in world blocks per noise cell. bigger
// scale = lower frequency = broader features. continentalness is the slowest
// because it draws whole continents; peaks/valleys is the fastest because it
// is the literal hills.

typedef struct {
    uint32_t seed;
    int      sea_level;        // engine WORLD_SEA_LEVEL, copied in

    // field frequencies, in cells (world units = world_xz / scale)
    float    cont_scale;       // continentalness
    float    eros_scale;       // erosion
    float    pv_scale;         // peaks & valleys
    float    weird_scale;      // weirdness, flips pv polarity in patches

    // octave counts per field
    int      cont_octaves;
    int      eros_octaves;
    int      pv_octaves;

    // domain warp strength applied to continentalness so coasts wobble
    float    cont_warp;

    // height assembly. final = base + cont_spline*cont_amp
    // + eros_factor*pv_spline*pv_amp
    float    cont_amp;         // vertical reach of the continent spline
    float    pv_amp;           // vertical reach of the peaks/valleys spline
    int      base_height;      // the y the continent spline is measured from

    // hard clamps so we never punch through the chunk roof or the floor
    int      min_height;
    int      max_height;

    // how aggressively erosion squashes relief. 0 = ignore erosion, 1 = full
    // flatten on the eroded end of the spline.
    float    erosion_bias;
} heightmap_params;

// sane defaults seeded from the world seed. tuned by eye against sea level 64
// and a 128-tall chunk; tweak and re-roll if the mountains scrape the sky.
heightmap_params heightmap_default_params(uint32_t seed, int sea_level);

#endif
