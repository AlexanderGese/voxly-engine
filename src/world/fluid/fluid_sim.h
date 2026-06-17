#ifndef WORLD_FLUID_FLUID_SIM_H
#define WORLD_FLUID_FLUID_SIM_H

#include "fluid_cell.h"
#include "source_block.h"
#include "fluid_interact.h"

// top-level driver that owns a grid + its sources and runs the whole pipeline
// each tick: stamp sources -> spread water -> spread lava -> resolve contact
// -> form new sources. this is the one thing the game layer needs to poke.

typedef struct voxl_fluid_sim {
    voxl_fluid_grid       grid;
    voxl_fluid_source_set sources;
    int   tick;
    bool  cold;          // biome flag, drives freezing
    int   last_changed;  // cells that moved on the most recent step
} voxl_fluid_sim;

void voxl_fluid_sim_init(voxl_fluid_sim *s);

// place / remove a source and seed the cell immediately.
int  voxl_fluid_sim_add_source(voxl_fluid_sim *s, int x, int y, int z, uint8_t kind);
bool voxl_fluid_sim_remove_source(voxl_fluid_sim *s, int x, int y, int z);

// mark a wall cell.
void voxl_fluid_sim_set_solid(voxl_fluid_sim *s, int x, int y, int z, bool solid);

// run one full tick. returns the number of cells that changed (0 = settled).
int  voxl_fluid_sim_step(voxl_fluid_sim *s);

// read a cell back (for the world layer to mirror into real blocks).
const voxl_fluid_cell *voxl_fluid_sim_cell(const voxl_fluid_sim *s,
                                           int x, int y, int z);

#endif
