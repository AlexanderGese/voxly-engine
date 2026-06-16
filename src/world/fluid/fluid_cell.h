#ifndef WORLD_FLUID_FLUID_CELL_H
#define WORLD_FLUID_FLUID_CELL_H

#include <stdint.h>
#include <stdbool.h>

// self-contained cellular-automaton fluid state. we dont touch the real
// chunk arrays here, instead the sim runs on its own little grid so it's
// easy to unit-test and reason about. the world glue lives elsewhere.

// kind of fluid in a cell. AIR means empty / not a fluid.
enum {
    VOXL_FLUID_AIR = 0,
    VOXL_FLUID_WATER,
    VOXL_FLUID_LAVA,
    VOXL_FLUID_KIND_COUNT
};

// max fluid amount in a single cell. 8 == a full block, like mc levels.
#define VOXL_FLUID_FULL    8
#define VOXL_FLUID_EMPTY   0

// one fluid cell. tiny on purpose so a grid stays cache friendly.
typedef struct {
    uint8_t kind;     // VOXL_FLUID_*
    uint8_t level;    // 0..VOXL_FLUID_FULL
    uint8_t falling;  // 1 if this is a falling column (full pressure down)
    uint8_t settled;  // 1 if it didnt change last step (skip work)
} voxl_fluid_cell;

// fixed cubic grid. N is small (sim region around the player or a test box).
#define VOXL_FLUID_GRID_N  16
#define VOXL_FLUID_GRID_VOL (VOXL_FLUID_GRID_N * VOXL_FLUID_GRID_N * VOXL_FLUID_GRID_N)

typedef struct voxl_fluid_grid {
    voxl_fluid_cell cells[VOXL_FLUID_GRID_VOL];
    // a parallel solid mask. 1 = wall, fluid cant enter. lets us bake in
    // terrain without needing the real block array.
    uint8_t solid[VOXL_FLUID_GRID_VOL];
} voxl_fluid_grid;

// flatten x/y/z -> index. y is the vertical axis.
int  voxl_fluid_index(int x, int y, int z);
bool voxl_fluid_in_bounds(int x, int y, int z);

void voxl_fluid_grid_clear(voxl_fluid_grid *g);

voxl_fluid_cell *voxl_fluid_at(voxl_fluid_grid *g, int x, int y, int z);
const voxl_fluid_cell *voxl_fluid_at_const(const voxl_fluid_grid *g, int x, int y, int z);

bool voxl_fluid_is_solid(const voxl_fluid_grid *g, int x, int y, int z);
void voxl_fluid_set_solid(voxl_fluid_grid *g, int x, int y, int z, bool solid);

// helpers used by the spread passes
bool voxl_fluid_cell_empty(const voxl_fluid_cell *c);
bool voxl_fluid_can_flow_into(const voxl_fluid_grid *g, int x, int y, int z, uint8_t kind);

#endif
