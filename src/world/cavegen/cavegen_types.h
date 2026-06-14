#ifndef WORLD_CAVEGEN_TYPES_H
#define WORLD_CAVEGEN_TYPES_H

#include <stdint.h>
#include "../block.h"
#include "../../config.h"
#include "../../math/vec3.h"

// shared plain-data for the cave generator. nothing in here touches a chunk or
// the renderer directly, its all "fill this scratch grid, then a carve pass
// stamps it into the world". keeps the algorithm testable in isolation, which
// i was very grateful for the first time the automata ate an entire chunk.

// how big a cave region we work on at once. one column of chunks, basically.
// y is full world height, x/z is a single chunk footprint plus a 1-cell skirt
// so neighbour caves line up across chunk borders without a seam.
#define CAVEGEN_PAD       1
#define CAVEGEN_DIM_X    (CHUNK_SIZE_X + 2 * CAVEGEN_PAD)
#define CAVEGEN_DIM_Z    (CHUNK_SIZE_Z + 2 * CAVEGEN_PAD)
#define CAVEGEN_DIM_Y     CHUNK_SIZE_Y
#define CAVEGEN_CELLS    (CAVEGEN_DIM_X * CAVEGEN_DIM_Y * CAVEGEN_DIM_Z)

// cell states in the scratch grid. we keep a couple of intermediate flavours
// so the connectivity pass can tell apart "carved by automata" from "carved by
// a worm" when it decides what to keep.
typedef enum {
    CAVEGEN_SOLID   = 0,   // untouched rock
    CAVEGEN_AIR     = 1,   // open space, generic
    CAVEGEN_WORM    = 2,   // open space dug by a worm tunnel
    CAVEGEN_FLOODED = 3,   // open but below the local water table
    CAVEGEN_SEALED  = 4    // carved then rejected by connectivity, refilled
} cavegen_cell;

// the knobs. all of this is per-world, seeded once and reused. defaults live in
// cavegen_default_params().
typedef struct {
    uint32_t seed;

    // cellular automata
    float    fill_chance;     // initial random fill prob for "solid"
    int      ca_iterations;   // smoothing passes
    int      ca_birth;        // >= this many solid neighbours -> become solid
    int      ca_survive;      // >= this many -> stay solid

    // 3d cheese noise gate. caves only allowed where density passes.
    float    cheese_scale;    // noise frequency
    float    cheese_thresh;   // [-1,1], lower = more cave

    // worms
    int      worm_count;      // worms seeded per region
    int      worm_max_steps;  // length cap per worm
    float    worm_radius;     // base tunnel radius
    float    worm_radius_var; // radius wobble amplitude
    float    worm_step;       // distance advanced each step
    float    worm_pitch_max;  // max vertical turn per step (radians)

    // vertical bounds. keep a crust under the surface and a floor over bedrock.
    int      min_y;           // no caves below this
    int      surface_margin;  // solid cells to leave under the heightmap
    int      water_table;     // cells below this that open up get flooded

    // connectivity
    int      min_region_cells;// pockets smaller than this get sealed back up
} cavegen_params;

// a worm's running state. its basically a drunk turtle in 3d.
typedef struct {
    vec3   pos;
    vec3   dir;        // unit heading
    float  radius;
    int    steps_left;
    uint32_t rng;      // private stream so worms dont desync the world rng
} cavegen_worm;

// region origin so cell coords map back to world coords. we offset by the pad.
typedef struct {
    int base_x;   // world x of cell (CAVEGEN_PAD,_,CAVEGEN_PAD)... see to_world
    int base_z;
    int chunk_cx;
    int chunk_cz;
} cavegen_origin;

cavegen_params cavegen_default_params(uint32_t seed);

#endif
