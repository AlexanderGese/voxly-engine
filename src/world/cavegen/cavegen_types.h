#ifndef WORLD_CAVEGEN_TYPES_H
#define WORLD_CAVEGEN_TYPES_H
#include <stdint.h>
#include "../block.h"
#include "../../config.h"
#include "../../math/vec3.h"
#define CAVEGEN_PAD       1
#define CAVEGEN_DIM_X    (CHUNK_SIZE_X + 2 * CAVEGEN_PAD)
#define CAVEGEN_DIM_Z    (CHUNK_SIZE_Z + 2 * CAVEGEN_PAD)
#define CAVEGEN_DIM_Y     CHUNK_SIZE_Y
#define CAVEGEN_CELLS    (CAVEGEN_DIM_X * CAVEGEN_DIM_Y * CAVEGEN_DIM_Z)
typedef enum {
    CAVEGEN_SOLID   = 0,   // untouched rock
    CAVEGEN_AIR     = 1,   // open space, generic
    CAVEGEN_WORM    = 2,   // open space dug by a worm tunnel
    CAVEGEN_FLOODED = 3,   // open but below the local water table
    CAVEGEN_SEALED  = 4    // carved then rejected by connectivity, refilled
} cavegen_cell;
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
#endif
