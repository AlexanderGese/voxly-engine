#ifndef WORLD_FLUID_FLUID_CELL_H
#define WORLD_FLUID_FLUID_CELL_H
#include <stdint.h>
#include <stdbool.h>
#define VOXL_FLUID_FULL    8
#define VOXL_FLUID_EMPTY   0
typedef struct {
    uint8_t kind;     // VOXL_FLUID_*
    uint8_t level;    // 0..VOXL_FLUID_FULL
    uint8_t falling;  // 1 if this is a falling column (full pressure down)
    uint8_t settled;  // 1 if it didnt change last step (skip work)
} voxl_fluid_cell;
#define VOXL_FLUID_GRID_N  16
#define VOXL_FLUID_GRID_VOL (VOXL_FLUID_GRID_N * VOXL_FLUID_GRID_N * VOXL_FLUID_GRID_N)
typedef struct voxl_fluid_grid {
    voxl_fluid_cell cells[VOXL_FLUID_GRID_VOL];
    // a parallel solid mask. 1 = wall, fluid cant enter. lets us bake in
    // terrain without needing the real block array.
    uint8_t solid[VOXL_FLUID_GRID_VOL];
} voxl_fluid_grid;
#endif
