#ifndef WORLD_FLUID_FLUID_PRESSURE_H
#define WORLD_FLUID_FLUID_PRESSURE_H
#include "fluid_cell.h"
// hydrostatic pressure. not used for the cellular spread (that's level based)
// but handy for gameplay: deeper water crushes harder, pushes doors, etc.
// pressure here is "how many full cells of fluid sit above this one".
// column depth above a cell of the same-or-any fluid, in cells.
int voxl_fluid_depth_above(const voxl_fluid_grid *g, int x, int y, int z);
// gauge pressure at a cell. rho * g * depth, with depth measured from the
// fluid surface. returns 0 in non-fluid cells.
float voxl_fluid_pressure_at(const voxl_fluid_grid *g, int x, int y, int z,
                             float gravity);
// horizontal pressure-gradient force on a cell, written into out_fx/out_fz.
// fluid wants to flow from high to low pressure; this is what drives currents.
void voxl_fluid_pressure_gradient(const voxl_fluid_grid *g, int x, int y, int z,
                                  float gravity, float *out_fx, float *out_fz);
#endif
