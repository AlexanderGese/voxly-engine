#ifndef WORLD_FLUID_FLUID_LEVEL_H
#define WORLD_FLUID_FLUID_LEVEL_H

#include "fluid_cell.h"

// helpers for dealing with fluid levels (0..8) and turning them into
// visual heights / fill fractions. kept separate so the spread code stays
// focused on movement and these stay focused on amounts.

// clamp a raw int level into the valid range.
int voxl_fluid_level_clamp(int level);

// add amount into a cell, returns the overflow that didnt fit.
int voxl_fluid_level_add(voxl_fluid_cell *c, uint8_t kind, int amount);

// remove amount, returns how much was actually removed. empties kind on 0.
int voxl_fluid_level_remove(voxl_fluid_cell *c, int amount);

// 0..1 fill fraction for rendering / collision. falling cells read as full.
float voxl_fluid_level_fraction(const voxl_fluid_cell *c);

// surface height in world units for a cell at integer y. a level-8 cell is
// ~0.9 high not 1.0, matching mc's slightly-sunk water top.
float voxl_fluid_surface_height(const voxl_fluid_cell *c, int y);

// average a 2x2 of corner samples into a smoothed corner height. used by
// the mesher to make sloped fluid tops.
float voxl_fluid_corner_height(float a, float b, float c, float d);

#endif
