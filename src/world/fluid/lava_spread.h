#ifndef WORLD_FLUID_LAVA_SPREAD_H
#define WORLD_FLUID_LAVA_SPREAD_H

#include "fluid_cell.h"

// lava behaves like water but slower and with a much shorter spread range.
// it also doesnt equalise as smoothly, it just creeps one level at a time.

// lava only reaches a few cells sideways. mc overworld lava is 3.
#define VOXL_FLUID_LAVA_RANGE  3

// lava ticks slower: only act every Nth call. caller passes a tick counter.
#define VOXL_FLUID_LAVA_PERIOD 4

// one lava step. returns cells changed.
int voxl_fluid_lava_step(voxl_fluid_grid *g);

// gated step: only advances on ticks where tick % period == 0.
int voxl_fluid_lava_step_gated(voxl_fluid_grid *g, int tick);

// returns true if a water cell is adjacent to this lava cell. used by the
// world layer to decide where lava turns to stone (cobble/obsidian).
bool voxl_fluid_lava_touches_water(const voxl_fluid_grid *g, int x, int y, int z);

#endif
