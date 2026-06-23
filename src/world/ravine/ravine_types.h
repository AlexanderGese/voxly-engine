#ifndef WORLD_RAVINE_TYPES_H
#define WORLD_RAVINE_TYPES_H

#include <stdint.h>
#include "../block.h"
#include "../../config.h"
#include "../../math/vec3.h"

// shared plain-data for the ravine + canyon carver. like the river pipeline
// this is heightfield-first: a region is a flat grid of cells and the whole
// thing reduces to "how far is this column from the canyon centreline, and how
// deep does the floor drop there". nothing in here touches a chunk or the gl
// side; only the final carve stage emits edits and even that is just a list.
//
// the difference from rivers is the path. a river follows water downhill, a
// ravine doesnt care about hydrology at all — it's a tectonic gash, so the
// centreline is an authored catmull-rom spline wandering across the region and
// the cross-section is a width falloff with exposed rock strata on the walls.
// i kept telling myself it was "just rivers with a different mask". it was not.

// a region is one chunk footprint plus a skirt so a ravine that clips the
// border lines up with the same ravine carved from the neighbour. the spline
// can bulge a fair bit so the skirt is wider than rivers'. 6 stopped the seams.
#define RAVINE_PAD        6
#define RAVINE_DIM_X     (CHUNK_SIZE_X + 2 * RAVINE_PAD)
#define RAVINE_DIM_Z     (CHUNK_SIZE_Z + 2 * RAVINE_PAD)
#define RAVINE_CELLS     (RAVINE_DIM_X * RAVINE_DIM_Z)

// a canyon centreline is built from this many control knots spanning the
// region. catmull-rom needs the two end knots as phantom tangents so the
// usable span is the inner ones, but we keep them all for simplicity.
#define RAVINE_MAX_KNOTS  10

// strata bands stacked from the floor up. each band is one rock type exposed
// on the canyon wall. 6 reads as geology without turning into a barcode.
#define RAVINE_MAX_STRATA 6

// what a cell resolved to after the mask pass. the carver reads this to decide
// what to do with the column. one byte, like the river version.
typedef enum {
    RAVINE_OUTSIDE = 0,   // beyond the falloff, untouched
    RAVINE_WALL    = 1,   // inside the falloff but above the floor: a sloped wall
    RAVINE_FLOOR   = 2,   // the cut floor of the canyon, near the centreline
    RAVINE_RIM     = 3     // the lip just outside the wall, gets a little erosion
} ravine_cellkind;

// one strata band. blocks between [y_lo, y_hi) on a wall get this id. bands are
// sorted floor-up and the top one is open-ended (y_hi acts as a soft cap).
typedef struct {
    int      y_lo;        // world y where this band starts
    int      y_hi;        // world y where the next band takes over
    block_id id;          // what's exposed in this band
    uint8_t  hardness;    // 0..255, biases the wall jitter — hard rock juts out
} ravine_band;

// the knobs. seeded once per world, reused for every region. defaults live in
// ravine_default_params(). canyons are rare and dramatic so most of these gate
// hard or shape the silhouette rather than the placement.
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

// a single emitted edit, world space. the driver stamps these wherever it
// likes. a wall column emits several (one per exposed band), a floor column one.
typedef struct {
    int      x, y, z;
    block_id id;
} ravine_edit;

ravine_params ravine_default_params(uint32_t seed, int sea_level);

// clamp helper used in a dozen places. inclusive both ends.
int ravine_clampi(int v, int lo, int hi);

// cell <-> region index. cells are pad-inclusive [0, RAVINE_DIM). callers that
// stray out of bounds get -1 and are expected to skip.
int ravine_cell_index(int cx, int cz);

#endif
