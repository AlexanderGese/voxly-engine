#ifndef WORLD_EROSION_TYPES_H
#define WORLD_EROSION_TYPES_H
#include <stdint.h>
#include "../../math/vec2.h"
#define EROSION_PAD       2
#define EROSION_DIM_X    (16 + 2 * EROSION_PAD)   // CHUNK_SIZE_X + skirt
#define EROSION_DIM_Z    (16 + 2 * EROSION_PAD)
#define EROSION_CELLS    (EROSION_DIM_X * EROSION_DIM_Z)
typedef struct {
    int   ox, oz;                  // world coord of cell (PAD,PAD)
    float height[EROSION_CELLS];   // terrain surface height, float blocks
    float hardness[EROSION_CELLS]; // erosion resistance, [0,1]
    float sediment[EROSION_CELLS]; // loose material sitting on top
} erosion_field;
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
#endif
