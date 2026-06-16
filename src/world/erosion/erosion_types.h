#ifndef WORLD_EROSION_TYPES_H
#define WORLD_EROSION_TYPES_H

#include <stdint.h>
#include "../../math/vec2.h"

// shared plain-data for the heightmap erosion pass. like cavegen this never
// touches a chunk or the renderer directly: worldgen hands us a float
// heightmap, we chew on it (rain droplets, talus slumping, sediment moving
// downhill) and hand back a new heightmap. the apply stage is the only thing
// that knows what a block is. keeps the nasty floating point math testable,
// which i needed the day the whole continent slid into the sea.

// a heightmap tile we erode in one go. one chunk footprint plus a skirt so
// droplets that run off an edge still see the neighbour slope and dont pile
// up a cliff right on the chunk border. y is unused here, this is a 2d field.
#define EROSION_PAD       2
#define EROSION_DIM_X    (16 + 2 * EROSION_PAD)   // CHUNK_SIZE_X + skirt
#define EROSION_DIM_Z    (16 + 2 * EROSION_PAD)
#define EROSION_CELLS    (EROSION_DIM_X * EROSION_DIM_Z)

// the float heightmap we operate on. heights are in world blocks but kept as
// float so droplets can carve sub-block amounts before we re-quantise on apply.
// hardness is per-cell rock resistance [0,1], 1 = bedrock-ish, barely erodes.
// sediment is the standing loose material that hasnt settled into height yet,
// the apply stage folds it back in. all three are flat row-major arrays.
typedef struct {
    int   ox, oz;                  // world coord of cell (PAD,PAD)
    float height[EROSION_CELLS];   // terrain surface height, float blocks
    float hardness[EROSION_CELLS]; // erosion resistance, [0,1]
    float sediment[EROSION_CELLS]; // loose material sitting on top
} erosion_field;

// the knobs. seeded once per world, reused for every tile. defaults live in
// erosion_default_params(). yes there are a lot of them. water is fussy.
typedef struct {
    uint32_t seed;

    // hydraulic (rain droplet) pass
    int    droplets;          // droplets spawned per tile
    int    droplet_lifetime;  // max steps before a droplet evaporates
    float  inertia;           // 0 = follow gradient exactly, 1 = keep momentum
    float  capacity_factor;   // how much sediment a droplet can hold per slope
    float  min_capacity;      // floor so flat water still moves a little
    float  deposit_rate;      // fraction of excess sediment dropped per step
    float  erode_rate;        // fraction of free capacity carved per step
    float  evaporate_rate;    // water lost per step, drives the carry capacity
    float  gravity;           // accelerates the droplet down slopes
    float  start_water;       // water volume a droplet spawns with
    float  start_speed;       // initial speed
    float  erode_radius;      // carve is smeared over this radius (cells)

    // thermal (talus) pass
    int    thermal_iters;     // relaxation sweeps
    float  talus_angle;       // max stable height delta between neighbours
    float  thermal_rate;      // fraction of the excess moved per sweep

    // sediment settling
    float  settle_thresh;     // below this, loose sediment becomes height
    int    settle_iters;
} erosion_params;

// a single rain droplet's running state. its a marble rolling down wet sand.
// pos is in continuous field space (cell units), not world blocks.
typedef struct {
    vec2   pos;
    vec2   dir;        // unit-ish heading, carries momentum between steps
    float  speed;
    float  water;      // shrinks via evaporation
    float  sediment;   // load currently carried
} erosion_droplet;

// result tallies from a tile pass, handy for tuning and the f3 overlay.
typedef struct {
    int   droplets_run;
    int   droplets_offmap;   // died by running off the skirt
    float total_eroded;      // sum of height removed
    float total_deposited;   // sum of height added back
    float max_delta;         // largest single-cell height change
} erosion_stats;

erosion_params erosion_default_params(uint32_t seed);

// flat index helpers. clamped variant keeps droplets from indexing the skirt.
static inline int erosion_idx(int x, int z) { return z * EROSION_DIM_X + x; }
int erosion_in_bounds(int x, int z);

#endif
