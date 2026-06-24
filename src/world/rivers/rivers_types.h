#ifndef WORLD_RIVERS_TYPES_H
#define WORLD_RIVERS_TYPES_H

#include <stdint.h>
#include "../block.h"
#include "../../config.h"
#include "../../math/vec3.h"

// shared plain-data for the river+lake carver. the whole pipeline is
// heightfield-first: we work on a 2d grid of surface heights, compute where
// water would flow, accumulate drainage, trace the wet channels downhill and
// pond up the closed basins. nothing in here touches a chunk or the renderer,
// the carve pass at the very end is the only thing that talks to the world.
//
// i went back and forth on doing this in 3d (like cavegen does) but rivers are
// fundamentally a surface phenomenon, so a tall stack of 2d passes ended up way
// cheaper and a lot easier to reason about. famous last words.

// a region is one chunk footprint plus a skirt so flow lines up across chunk
// borders. the skirt has to be wide enough that an edge cell can see where its
// water is heading, otherwise rivers get amputated at the seam. 4 felt safe.
#define RIVERS_PAD        4
#define RIVERS_DIM_X     (CHUNK_SIZE_X + 2 * RIVERS_PAD)
#define RIVERS_DIM_Z     (CHUNK_SIZE_Z + 2 * RIVERS_PAD)
#define RIVERS_CELLS     (RIVERS_DIM_X * RIVERS_DIM_Z)

// d8 flow directions. index packs into the dir grid as a single byte. the
// order matters: rivers_flow_dx/dz are indexed by this, and RIVERS_DIR_NONE is
// a sentinel for pits and out-of-region drains.
typedef enum {
    RIVERS_DIR_E  = 0,
    RIVERS_DIR_NE = 1,
    RIVERS_DIR_N  = 2,
    RIVERS_DIR_NW = 3,
    RIVERS_DIR_W  = 4,
    RIVERS_DIR_SW = 5,
    RIVERS_DIR_S  = 6,
    RIVERS_DIR_SE = 7,
    RIVERS_DIR_NONE = 8
} rivers_dir;

// what a cell ended up as after the wet passes ran. the carve stage reads this
// to decide what to stamp. kept deliberately small, one byte.
typedef enum {
    RIVERS_DRY    = 0,   // land, untouched
    RIVERS_RIVER  = 1,   // flowing channel, carved to a bed
    RIVERS_LAKE   = 2,   // ponded water in a filled basin
    RIVERS_BANK   = 3,   // land adjacent to water, gets a sand lip
    RIVERS_SOURCE = 4    // spring head, where a river first crosses threshold
} rivers_wet;

// the knobs. seeded once per world, reused for every region. defaults in
// rivers_default_params().
typedef struct {
    uint32_t seed;

    int   sea_level;        // engine WORLD_SEA_LEVEL, passed in

    // flow accumulation. each cell starts with 1 unit of "rain" and pushes it
    // downhill; a cell becomes a river once it has collected this much.
    float rain_per_cell;    // base water dropped on every land cell
    float rain_jitter;      // hashed +/- variation so basins arent uniform
    int   river_threshold;  // accumulated units needed to call it a river

    // channel shaping
    int   max_depth;        // deepest we will cut a river bed below surface
    float depth_per_log;    // bed deepens with log2(accumulation)
    int   bank_width;       // cells of bank flagged either side of a channel

    // lake / depression fill (priority flood)
    int   fill_max_y;       // never pond water above this (keeps mountains dry)
    int   min_lake_cells;   // puddles smaller than this get left as dry pits
    int   carve_outlet;     // 1 = notch a spillway where a full lake overflows

    // tracing
    int   max_trace_steps;  // safety cap so a flat loop cant spin forever
    int   min_source_y;     // springs below this are suppressed (underwater)
} rivers_params;

// a single emitted edit. the driver stamps these into the world wherever it
// likes. y is world space. one cell can emit several (bed, water column, lip).
typedef struct {
    int      x, y, z;
    block_id id;
} rivers_cell;

// a traced river vertex. produced by the tracer, consumed by the carver. carries
// the running accumulation so the carver knows how wide/deep to cut.
typedef struct {
    int   x, z;          // cell coords (pad-inclusive)
    int   surface_y;     // land height here before carving
    float accum;         // flow accumulation at this vertex
    int   water_y;       // resolved water surface height (filled in by trace)
} rivers_vertex;

rivers_params rivers_default_params(uint32_t seed, int sea_level);

// direction step tables. defined in rivers_flow.c, declared here so the field
// and trace passes can share them without dragging in the whole flow header.
extern const int rivers_dir_dx[8];
extern const int rivers_dir_dz[8];

// reverse a d8 direction (E<->W etc). NONE maps to NONE.
rivers_dir rivers_dir_opposite(rivers_dir d);

#endif
