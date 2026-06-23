#ifndef WORLD_RAVINE_TYPES_H
#define WORLD_RAVINE_TYPES_H
#include <stdint.h>
#include "../block.h"
#include "../../config.h"
#include "../../math/vec3.h"
#define RAVINE_PAD        6
#define RAVINE_DIM_X     (CHUNK_SIZE_X + 2 * RAVINE_PAD)
#define RAVINE_DIM_Z     (CHUNK_SIZE_Z + 2 * RAVINE_PAD)
#define RAVINE_CELLS     (RAVINE_DIM_X * RAVINE_DIM_Z)
#define RAVINE_MAX_KNOTS  10
#define RAVINE_MAX_STRATA 6
typedef enum {
    RAVINE_OUTSIDE = 0,   // beyond the falloff, untouched
    RAVINE_WALL    = 1,   // inside the falloff but above the floor: a sloped wall
    RAVINE_FLOOR   = 2,   // the cut floor of the canyon, near the centreline
    RAVINE_RIM     = 3     // the lip just outside the wall, gets a little erosion
} ravine_cellkind;
typedef struct {
    int      y_lo;        // world y where this band starts
    int      y_hi;        // world y where the next band takes over
    block_id id;          // what's exposed in this band
    uint8_t  hardness;    // 0..255, biases the wall jitter — hard rock juts out
} ravine_band;
typedef struct {
    uint32_t seed;
    int   sea_level;       // engine WORLD_SEA_LEVEL, passed in

    // placement. a ravine only exists in a region whose hashed roll clears the
    // gate. lower rarity = more ravines. the default is deliberately stingy.
    int   rarity;          // 1 in N regions seeds a ravine
    int   min_floor_y;     // never cut the floor below this (bedrock guard)

    // silhouette
    int   max_depth;       // deepest the floor drops below the local surface
    float half_width;      // half-width of the floor at the widest point
    float wall_slope;      // blocks of horizontal run per block of wall rise
    float width_wobble;    // 0..1, how much the half-width breathes along length

    // the catmull-rom path
    int   knot_span;       // cells between successive control knots
    float knot_jitter;     // lateral wander of each knot, in cells
    float drift;           // net sideways bias so canyons arc instead of zigzag

    // wall detailing
    float wall_jitter;     // perlin amplitude that roughens the wall face
    int   strata_jitter;   // +/- y wobble applied to band boundaries
    int   expose_ores;     // 1 = let oregen-style flecks show on the walls
} ravine_params;
#endif
